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
        # The graph. Nodes may be removed or replaced in place after add(); everything derived
        # from them (e.g. which node produces a collection) is recomputed when used.
        self.nodes: list[Node] = []
        self.deferred_sections: list[tuple] = []
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
    def _producers(self, nodes: list[Node] | None = None) -> dict[str, tuple[Node, str]]:
        """Output collection name -> (producing node, type), from `nodes` (default: the graph)."""
        producers = {}
        for node in self.nodes if nodes is None else nodes:
            for name, typ in zip(node.outputs, node.output_types):
                if name in producers:
                    raise GraphError(f"{node.prefix}: output '{name}' is already produced by "
                                     f"{producers[name][0].prefix}")
                producers[name] = (node, typ)
        return producers

    def _signature(self, ftype):
        if ftype.name not in self._signatures:
            self._signatures[ftype.name] = ftype.signature()
            self._defaults[ftype.name] = ftype.config_defaults()
        return self._signatures[ftype.name], self._defaults[ftype.name]

    def add(self, ftype, tag: str, inputs: list[str], outputs: list[str], **config) -> Node:
        """Add one factory wiring (the Python equivalent of JOmniFactoryGeneratorT<T>)."""
        if self._plugin is None:
            raise GraphError(f"{tag}: add factories inside 'with app.plugin(name):'")
        node = self._make_node(self._plugin, ftype, tag, inputs, outputs, config)
        self._register(node)
        return node

    def deferred(self, function) -> None:
        """Add a graph section that needs services, such as geometry constants.

        `function(graph)` runs while JANA generates factories (inside JApplication::Initialize),
        with a DeferredGraph that offers add() as well as geometry and parameter queries. Use it
        only for decisions that depend on the geometry: its nodes are checked when it runs, not
        when the graph is configured.
        """
        if self._plugin is None:
            raise GraphError("add deferred sections inside 'with app.plugin(name):'")
        self.deferred_sections.append((self._plugin, function))

    def _make_node(self, plugin, ftype, tag, inputs, outputs, config) -> Node:
        node = Node(plugin, tag, ftype, list(inputs), list(outputs), dict(config))
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
        return node

    def _register(self, node: Node, nodes: list[Node] | None = None) -> None:
        """Append `node` to `nodes` (default: the graph) if its outputs are not produced yet."""
        nodes = self.nodes if nodes is None else nodes
        producers = self._producers(nodes)
        for name in node.outputs:
            if name in producers:
                raise GraphError(
                    f"{node.prefix}: output '{name}' is already produced by "
                    f"{producers[name][0].prefix}"
                )
        nodes.append(node)

    # -- checks ----------------------------------------------------------------------------
    def check(self, source_collections: Iterable[str] | None = None) -> list[str]:
        """Check that every input has a producer of the right type.

        Inputs without a producer in the Python graph are returned as 'external'. They are
        expected from the event source or from C++ plugins. If `source_collections` is
        given, an external input that is not in it raises GraphError, unless C++ plugins
        that may produce it are loaded (then it is only reported).
        """
        producers = self._producers()
        external = []
        errors = []
        for node in self.nodes:
            for name, typ in zip(node.inputs, node.input_types):
                if name in producers:
                    _, produced = producers[name]
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
                close = {n: difflib.get_close_matches(n, known | set(producers), n=1)
                         for n in missing}
                msg = ", ".join(f"'{n}'" + (f" (did you mean '{close[n][0]}'?)" if close[n] else "")
                                for n in missing)
                errors.append(f"inputs not produced by the graph or the source: {msg}")
        if errors:
            raise GraphError("\n".join(errors))
        return external

    def to_dot(self) -> str:
        """Graphviz rendering of the Python part of the graph."""
        producers = self._producers()
        lines = ["digraph eicrecon {", "  rankdir=LR;", "  node [shape=box];"]
        for node in self.nodes:
            lines.append(f'  "{node.prefix}" [label="{node.tag}\\n{node.type.name}"];')
            for name in node.inputs:
                src = producers.get(name)
                if src:
                    lines.append(f'  "{src[0].prefix}" -> "{node.prefix}" [label="{name}"];')
                else:
                    lines.append(f'  "{name}" [shape=ellipse];')
                    lines.append(f'  "{name}" -> "{node.prefix}";')
        lines.append("}")
        return "\n".join(lines)

    # -- execution -------------------------------------------------------------------------
    def build(self, load_config_file: str = "", plugin_paths: Iterable[str] = ()) -> _core.Application:
        """Create the JApplication and hand it the plugins and factories."""
        self.check()
        params = dict(self.params)
        # Defaults of the eicrecon executable, unless set with -P
        params.setdefault("jana:timeout", "180")
        params.setdefault("jana:warmup_timeout", "180")
        plugins = list(self.plugins)
        if params.get("plugins"):
            plugins += [p for p in params["plugins"].split(",") if p and p not in plugins]
        params["plugins"] = ",".join(plugins)
        core = _core.Application(params, self.event_sources, load_config_file, list(plugin_paths))
        # Defaults of the eicrecon executable, unless set on the command line or in a config file
        core.set_default_parameter("jana:timeout", "180")
        core.set_default_parameter("jana:warmup_timeout", "180")
        # plugins_to_ignore may also come from the config file, which is only read now: skip
        # the Python factories of ignored plugins, as JANA skips ignored C++ plugins
        ignored = set((core.get_parameter("plugins_to_ignore") or "").split(","))
        for node in self.nodes:
            if node.plugin in ignored:
                continue
            core.add_factory(node.type, node.plugin, node.tag, node.inputs, node.outputs,
                             node.config)
        # Deferred sections add their nodes to a per-build copy of the graph: duplicate outputs
        # are still detected, and the graph itself stays unchanged for later builds
        built = list(self.nodes)
        for plugin, function in self.deferred_sections:
            if plugin in ignored:
                continue
            core.add_deferred(plugin, _DeferredRunner(self, built, plugin, function))
        return core

    def capture_wiring(self, load_config_file: str = "",
                       plugin_paths: Iterable[str] = ()) -> list[dict]:
        """Initialize the job and return every factory wiring that was instantiated: from Python
        nodes and from C++ plugins alike, with effective plugin names and config values."""
        from .factories import import_all
        import_all()
        core = self.build(load_config_file, plugin_paths)
        capture = _core.WiringCapture()
        try:
            capture.start()
            core.initialize()
        finally:
            capture.stop()
            records = capture.records()
            core.close()
        return sorted(records, key=lambda r: (r["plugin"], r["tag"]))

    def run(self, mode: str = "run", load_config_file: str = "",
            dump_config_file: str = "jana.config", plugin_paths: Iterable[str] = (),
            benchmark: bool = False) -> int:
        """Run or inspect the job.

        mode: 'run', 'benchmark', 'dump-configs' (write all parameters to `dump_config_file`),
        'configs' (print all parameters) or 'list-factories' (print all factories).
        benchmark: with mode 'configs', also list the benchmarking parameters (eicrecon -c -b).
        """
        if mode == "list-factories":
            # quiet plugin loading, as eicrecon -L does
            self.params.setdefault("log:off", "JPluginLoader,JArrowProcessingController,JArrow")
        core = self.build(load_config_file, plugin_paths)
        try:
            if mode == "configs":
                core.initialize()
                if benchmark:
                    core.register_benchmark_parameters()
                print_table(["Name", "Value", "Description"], core.parameters(), max_width=[0, 32, 0])
                return 0
            if mode == "list-factories":
                core.initialize()
                print("\nList all the factories:\n")
                print_table(["Plugin", "Object name", "Tag"], core.factories())
                if core.get_parameter("podio:print_type_table") in ("1", "true", "True"):
                    core.open_podio_sources()
                return 0
            if mode == "run":
                # unknown parameters are errors when running (as in eicrecon)
                core.set_default_parameter("jana:parameter_strictness", "2")
            return core.execute(mode, dump_config_file)
        finally:
            core.close()


def print_table(header, rows, max_width=None):
    """Print rows as an aligned text table. Columns wider than max_width[i] (if > 0) do not
    widen the column, but are printed in full."""
    rows = [tuple(str(c) for c in r) for r in rows]
    widths = []
    for i, h in enumerate(header):
        cells = [len(r[i]) for r in rows]
        if max_width and max_width[i] > 0:
            cells = [c for c in cells if c <= max_width[i]]
        widths.append(max([len(h)] + cells))
    def line(cells):
        out = []
        for i, c in enumerate(cells):
            out.append(c if i == len(cells) - 1 else c.ljust(widths[i]))
        return " | ".join(out).rstrip()
    print(line(header))
    print("-+-".join("-" * w for w in widths))
    for r in rows:
        print(line(r))


class _DeferredRunner:
    """Called by PyDeferredGenerator with a _core.DeferredContext"""

    def __init__(self, app, nodes, plugin, function):
        self.app, self.nodes, self.plugin, self.function = app, nodes, plugin, function

    def __call__(self, ctx):
        self.function(DeferredGraph(self.app, self.nodes, self.plugin, ctx))


class DeferredGraph:
    """What a deferred graph section sees: add() with the usual checks, plus services."""

    def __init__(self, app, nodes, plugin, ctx):
        self._app, self._nodes, self.plugin, self.ctx = app, nodes, plugin, ctx

    def add(self, ftype, tag: str, inputs: list[str], outputs: list[str], **config) -> Node:
        node = self._app._make_node(self.plugin, ftype, tag, inputs, outputs, config)
        self._app._register(node, self._nodes)
        self.ctx.add_factory(ftype, tag, node.inputs, node.outputs, node.config)
        return node

    def constant_int(self, name):
        """Integer constant from the geometry, or None"""
        return self.ctx.constant_int(name)

    def constant_double(self, name):
        """Floating-point constant from the geometry (dd4hep units), or None"""
        return self.ctx.constant_double(name)

    def readout_segmentation_type(self, readout):
        """Segmentation type name of a readout, or None"""
        return self.ctx.readout_segmentation_type(readout)

    def parameter(self, key, default, description=""):
        """Register a string parameter with a default (overridable with -P) and return it"""
        return self.ctx.parameter(key, default, description)
