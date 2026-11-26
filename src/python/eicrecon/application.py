# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Algorithm graph description and execution.

The graph is plain Python data until :meth:`Application.run`: nodes are recorded and checked
(config field names and types, input/output counts, duplicate outputs, dangling inputs, edge
types) at configure time, and only then is a JApplication created and the factories handed
to JANA.
"""

from __future__ import annotations

import contextlib
import difflib
from dataclasses import dataclass, field
from typing import Iterable

from . import _core


class GraphError(Exception):
    """Raised for inconsistent graph definitions, before anything runs."""


@dataclass
class Node:
    plugin: str
    tag: str
    type: _core.FactoryType
    inputs: list[str]
    outputs: list[str]
    config: dict = field(default_factory=dict)
    input_types: list[str] = field(default_factory=list)
    output_types: list[str] = field(default_factory=list)

    @property
    def prefix(self) -> str:
        """Parameter prefix, as in C++: '<plugin>:<tag>'"""
        return f"{self.plugin}:{self.tag}" if self.plugin else self.tag


def _expand(slots, names, what, prefix):
    """Map collection names to (type, variadic) slots like JOmniFactory::PreInit does."""
    n_variadic = sum(1 for _, v in slots if v)
    n_fixed = len(slots) - n_variadic
    if n_variadic == 0:
        if len(names) != len(slots):
            raise GraphError(
                f"{prefix}: expected {len(slots)} {what} collection names, got {len(names)}"
            )
        return [t for t, _ in slots]
    n_var_names = len(names) - n_fixed
    if n_var_names < 0 or n_var_names % n_variadic:
        raise GraphError(
            f"{prefix}: {len(names)} {what} collection names cannot be distributed over "
            f"{n_fixed} fixed and {n_variadic} variadic slots"
        )
    per = n_var_names // n_variadic
    types = []
    for t, v in slots:
        types += [t] * (per if v else 1)
    return types


class Application:
    """An EICrecon job: parameters, C++ plugins (services, sources, writers) and factories."""

    def __init__(self, params: dict | None = None, event_sources: Iterable[str] = ()):
        self.params: dict[str, str] = {k: str(v) for k, v in (params or {}).items()}
        self.event_sources: list[str] = list(event_sources)
        self.plugins: list[str] = []
        self.nodes: list[Node] = []
        self._producers: dict[str, tuple[Node, str]] = {}
        self._plugin: str | None = None
        self._signatures: dict[str, dict] = {}
        self._defaults: dict[str, dict] = {}

    # -- plugins ---------------------------------------------------------------------------
    def add_plugin(self, name: str) -> None:
        """Load a C++ JANA plugin (services, event sources, writers, unported detectors)."""
        if name not in self.plugins:
            self.plugins.append(name)

    @contextlib.contextmanager
    def plugin(self, name: str):
        """Group the factories added in this block under plugin `name`.

        The plugin name is part of the parameter prefix ('<plugin>:<tag>:<param>') and of the
        algorithm name used to seed per-event random numbers, so it must match the C++ plugin
        name for results and -P overrides to be identical.
        """
        previous, self._plugin = self._plugin, name
        try:
            yield self
        finally:
            self._plugin = previous

    # -- factories -------------------------------------------------------------------------
    def _signature(self, ftype):
        if ftype.name not in self._signatures:
            self._signatures[ftype.name] = ftype.signature()
            self._defaults[ftype.name] = ftype.config_defaults()
        return self._signatures[ftype.name], self._defaults[ftype.name]

    def add(self, ftype, tag: str, inputs: list[str], outputs: list[str], **config) -> Node:
        """Add one factory wiring (the Python equivalent of JOmniFactoryGeneratorT<T>)."""
        if self._plugin is None:
            raise GraphError(f"{tag}: add factories inside 'with app.plugin(name):'")
        node = Node(self._plugin, tag, ftype, list(inputs), list(outputs), dict(config))
        sig, defaults = self._signature(ftype)

        for key in config:
            if key not in defaults:
                hint = difflib.get_close_matches(key, defaults.keys(), n=1)
                raise GraphError(
                    f"{node.prefix}: {ftype.name} has no config field '{key}'"
                    + (f" (did you mean '{hint[0]}'?)" if hint else "")
                )
        try:
            ftype.validate_config(node.config, node.prefix)
        except (TypeError, KeyError) as e:
            raise GraphError(str(e)) from None

        node.input_types = _expand(sig["inputs"], node.inputs, "input", node.prefix)
        node.output_types = _expand(sig["outputs"], node.outputs, "output", node.prefix)

        for name, typ in zip(node.outputs, node.output_types):
            if name in self._producers:
                other = self._producers[name][0]
                raise GraphError(
                    f"{node.prefix}: output '{name}' is already produced by {other.prefix}"
                )
            self._producers[name] = (node, typ)
        self.nodes.append(node)
        return node

    # -- checks ----------------------------------------------------------------------------
    def check(self, source_collections: Iterable[str] | None = None) -> list[str]:
        """Check that every input has a producer of the right type.

        Inputs without a producer in the Python graph are returned as 'external'. They are
        expected from the event source or from C++ plugins. If `source_collections` is
        given, an external input that is not in it raises GraphError, unless C++ plugins
        that may produce it are loaded (then it is only reported).
        """
        external = []
        errors = []
        for node in self.nodes:
            for name, typ in zip(node.inputs, node.input_types):
                if name in self._producers:
                    _, produced = self._producers[name]
                    if produced != typ:
                        errors.append(
                            f"{node.prefix}: input '{name}' has type {typ}, "
                            f"but is produced as {produced}"
                        )
                elif name not in external:
                    external.append(name)
        if source_collections is not None:
            known = set(source_collections)
            missing = [n for n in external if n not in known]
            if missing:
                close = {n: difflib.get_close_matches(n, known | set(self._producers), n=1)
                         for n in missing}
                msg = ", ".join(f"'{n}'" + (f" (did you mean '{close[n][0]}'?)" if close[n] else "")
                                for n in missing)
                errors.append(f"inputs not produced by the graph or the source: {msg}")
        if errors:
            raise GraphError("\n".join(errors))
        return external

    def to_dot(self) -> str:
        """Graphviz rendering of the Python part of the graph."""
        lines = ["digraph eicrecon {", "  rankdir=LR;", "  node [shape=box];"]
        for node in self.nodes:
            lines.append(f'  "{node.prefix}" [label="{node.tag}\\n{node.type.name}"];')
            for name in node.inputs:
                src = self._producers.get(name)
                if src:
                    lines.append(f'  "{src[0].prefix}" -> "{node.prefix}" [label="{name}"];')
                else:
                    lines.append(f'  "{name}" [shape=ellipse];')
                    lines.append(f'  "{name}" -> "{node.prefix}";')
        lines.append("}")
        return "\n".join(lines)

    # -- execution -------------------------------------------------------------------------
    def build(self) -> _core.Application:
        """Create the JApplication and hand it the plugins and factories."""
        self.check()
        params = dict(self.params)
        plugins = list(self.plugins)
        if params.get("plugins"):
            plugins += [p for p in params["plugins"].split(",") if p and p not in plugins]
        params["plugins"] = ",".join(plugins)
        core = _core.Application(params, self.event_sources)
        for node in self.nodes:
            core.add_factory(node.type, node.plugin, node.tag, node.inputs, node.outputs,
                             node.config)
        return core

    def run(self, mode: str = "run") -> int:
        """Run (or, with mode='configs'/'list-factories'/'list-collections', inspect) the job."""
        core = self.build()
        try:
            return core.execute(mode)
        finally:
            core.close()
