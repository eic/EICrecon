# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""eicrecon-py: command-line frontend (prototype replacement for the eicrecon executable).

    eicrecon-py [-Pkey=value ...] [--graph FILE_OR_MODULE] input.edm4hep.root ...

The graph is built by calling ``configure(app)`` from --graph (a Python file or module,
default: eicrecon.default). -P parameters are applied with the same precedence as in
eicrecon: they override the defaults set in the graph definition.
"""

import argparse
import importlib
import importlib.util
import os
import sys


def _load_graph(spec):
    if spec.endswith(".py") or os.path.sep in spec:
        mod_spec = importlib.util.spec_from_file_location("eicrecon_user_graph", spec)
        module = importlib.util.module_from_spec(mod_spec)
        mod_spec.loader.exec_module(module)
        return module
    return importlib.import_module(spec)


def parse_args(argv):
    p = argparse.ArgumentParser(prog="eicrecon-py", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("inputs", nargs="*", help="input files (event sources)")
    p.add_argument("-P", dest="params", action="append", default=[], metavar="KEY=VALUE",
                   help="set a JANA parameter (repeatable), e.g. -Pnthreads=4")
    p.add_argument("--graph", default="eicrecon.default",
                   help="Python file or module defining configure(app)")
    p.add_argument("--use-cpp", action="append", default=[], metavar="PLUGIN",
                   help="load this detector from its C++ plugin instead of its Python port")
    mode = p.add_mutually_exclusive_group()
    mode.add_argument("-c", "--configs", dest="mode", action="store_const", const="configs",
                      help="print all parameters and exit")
    mode.add_argument("-L", "--list-factories", dest="mode", action="store_const",
                      const="list-factories", help="list all components and exit")
    mode.add_argument("--list-collections", dest="mode", action="store_const",
                      const="list-collections", help="list all collections and exit")
    mode.add_argument("--dump-graph", dest="mode", action="store_const", const="dump-graph",
                      help="print the Python part of the graph (Graphviz) and exit")
    mode.add_argument("--check", dest="mode", action="store_const", const="check",
                      help="check the graph and exit")
    p.add_argument("--list-default-plugins", action="store_true")
    args = p.parse_args(argv)
    params = {}
    for kv in args.params:
        if "=" not in kv:
            p.error(f"-P{kv}: expected KEY=VALUE")
        k, v = kv.split("=", 1)
        params[k] = v
    args.params = params
    return args


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)

    from . import Application, GraphError
    from .default import DEFAULT_PLUGINS
    from .detectors import PORTED

    if args.list_default_plugins:
        for name in DEFAULT_PLUGINS:
            print(f"  {name}" + ("  (python)" if name in PORTED else ""))
        return 0

    app = Application(args.params, args.inputs)
    graph = _load_graph(args.graph)
    try:
        if args.graph == "eicrecon.default":
            graph.configure(app, use_cpp=args.use_cpp)
        else:
            graph.configure(app)
        external = app.check()
    except GraphError as e:
        print(f"eicrecon-py: graph error:\n{e}", file=sys.stderr)
        return 2

    if args.mode == "dump-graph":
        print(app.to_dot())
        return 0
    if args.mode == "check":
        print(f"{len(app.nodes)} Python factories, {len(app.plugins)} C++ plugins")
        print("inputs expected from source or C++ plugins:", ", ".join(external))
        return 0
    return app.run(args.mode or "run")


if __name__ == "__main__":
    sys.exit(main())
