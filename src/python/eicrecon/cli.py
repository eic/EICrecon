# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""eicrecon-py: command-line frontend with the options of the eicrecon executable.

The graph is built by calling ``configure(app)`` from --graph (a Python file or module,
default: eicrecon.default). -P parameters override the defaults set in the graph definition,
with the same precedence as in eicrecon.
"""

import importlib
import importlib.util
import os
import sys

USAGE = """\
Usage:
    eicrecon-py [options] source1 source2 ...

Description:
    Reads events from the sources and reconstructs them with the algorithm graph defined in
    Python (default: the eicrecon.default module). Command-line flags control configuration.

Options:
   -h   --help                  Display this message
   -v   --version               Display version information
   -j   --janaversion           Display JANA version information
   -c   --configs               Display configuration parameters
   -l   --loadconfigs <file>    Load configuration parameters from file
   -d   --dumpconfigs <file>    Dump configuration parameters to file
   -b   --benchmark             Run in benchmark mode
   -L   --list-factories        List all the factories without running
   -Pkey=value                  Specify a configuration parameter
   -Pplugin:param=value         Specify a parameter value for a plugin

   --list-default-plugins       List all the default plugins
   --list-available-plugins     List plugins at $JANA_PLUGIN_PATH and $EICrecon_MY

   -Pplugins=values             Comma-separated list of extra plugins to load
   -Pplugins_to_ignore=values   Comma-separated list of plugins to ignore
                                ('default' ignores all default plugins)

Python graph options:
   --graph <file or module>     Python file or module defining configure(app)
                                (default: eicrecon.default)
   --use-cpp <plugin>           Load a ported detector from its C++ plugin instead
   --check                      Check the graph and exit
   --dump-graph                 Print the Python part of the graph (Graphviz) and exit

Example:
    eicrecon-py -Pplugins=plugin1,plugin2,plugin3 -Pnthreads=8 infile.root
    eicrecon-py --graph mygraph.py -Ppodio:output_file=out.edm4eic.root infile.root
"""

FLAGS = {
    "-h": "help", "--help": "help",
    "-v": "version", "--version": "version",
    "-j": "janaversion", "--janaversion": "janaversion",
    "-c": "configs", "--configs": "configs",
    "-l": "loadconfigs", "--loadconfigs": "loadconfigs",
    "-d": "dumpconfigs", "--dumpconfigs": "dumpconfigs",
    "-b": "benchmark", "--benchmark": "benchmark",
    "-L": "list-factories", "--list-factories": "list-factories",
    "--list-default-plugins": "list-default-plugins",
    "--list-available-plugins": "list-available-plugins",
    "--check": "check",
    "--dump-graph": "dump-graph",
}
WITH_OPTIONAL_FILE = {"loadconfigs", "dumpconfigs"}
WITH_VALUE = {"--graph": "graph", "--use-cpp": "use_cpp"}


class UsageError(Exception):
    pass


class Options:
    def __init__(self):
        self.flags = set()
        self.params = {}
        self.sources = []
        self.load_config_file = ""
        self.dump_config_file = ""
        self.graph = "eicrecon.default"
        self.use_cpp = []


def parse_args(argv):
    """Parse eicrecon-style arguments: flags, -Pkey=value, and event sources."""
    opts = Options()
    if not argv:
        opts.flags.add("help")
    i = 0
    while i < len(argv):
        arg = argv[i]
        if not arg.startswith("-"):
            opts.sources.append(arg)
        elif arg in FLAGS:
            flag = FLAGS[arg]
            opts.flags.add(flag)
            if flag in WITH_OPTIONAL_FILE:
                name = "jana.config"
                if i + 1 < len(argv) and not argv[i + 1].startswith("-"):
                    name = argv[i + 1]
                    i += 1
                if flag == "loadconfigs":
                    opts.load_config_file = name
                else:
                    opts.dump_config_file = name
        elif arg in WITH_VALUE or arg.split("=", 1)[0] in WITH_VALUE:
            key, _, value = arg.partition("=")
            if not value:
                if i + 1 >= len(argv):
                    raise UsageError(f"{key} needs a value")
                value = argv[i + 1]
                i += 1
            if WITH_VALUE[key] == "use_cpp":
                opts.use_cpp.append(value)
            else:
                opts.graph = value
        elif arg.startswith("-P"):
            key, eq, value = arg[2:].partition("=")
            if not key or not eq:
                raise UsageError(f"Invalid JANA parameter '{arg}': Expected format -Pkey=value")
            if key in opts.params:
                print(f"Duplicate parameter '{arg}' ignored")
            else:
                opts.params[key] = value
        else:
            raise UsageError(f"Invalid command line flag '{arg}'")
        i += 1
    return opts


def available_plugins(default_plugins):
    """Default plugins plus every *.so in $JANA_PLUGIN_PATH and $EICrecon_MY/plugins."""
    names = set(default_plugins)
    dirs = [d for d in os.environ.get("JANA_PLUGIN_PATH", "").split(":") if d]
    if os.environ.get("EICrecon_MY"):
        dirs.append(os.path.join(os.environ["EICrecon_MY"], "plugins"))
    for d in dirs:
        if os.path.isdir(d):
            names.update(f[:-3] for f in os.listdir(d) if f.endswith(".so"))
    return sorted(names)


def load_graph(spec):
    if spec.endswith(".py") or os.path.sep in spec:
        mod_spec = importlib.util.spec_from_file_location("eicrecon_user_graph", spec)
        if mod_spec is None:
            raise UsageError(f"cannot load graph file '{spec}'")
        module = importlib.util.module_from_spec(mod_spec)
        mod_spec.loader.exec_module(module)
        return module
    return importlib.import_module(spec)


def main(argv=None):
    try:
        opts = parse_args(sys.argv[1:] if argv is None else argv)
    except UsageError as e:
        print(e, file=sys.stderr)
        print(USAGE, file=sys.stderr)
        return 2

    if "help" in opts.flags:
        print(USAGE)
        return 0

    from . import Application, GraphError, _core
    from .application import print_table
    from .default import DEFAULT_PLUGINS
    from .detectors import PORTED

    if "version" in opts.flags:
        print(f"EICrecon version: {_core.eicrecon_version}")
        return 0
    if "janaversion" in opts.flags:
        print(f"JANA version: {_core.jana_version}")
        return 0
    if "list-default-plugins" in opts.flags:
        print("\n List default plugins:\n")
        print_table(["Plugin name", "Wiring"],
                    [(p, "python" if p in PORTED else "C++") for p in DEFAULT_PLUGINS])
        return 0
    if "list-available-plugins" in opts.flags:
        print("\n List available plugins:\n")
        print_table(["Plugin name"], [(p,) for p in available_plugins(DEFAULT_PLUGINS)])
        return 0

    app = Application(opts.params, opts.sources)
    try:
        graph = load_graph(opts.graph)
        if opts.graph == "eicrecon.default":
            graph.configure(app, use_cpp=opts.use_cpp)
        else:
            graph.configure(app)
        external = app.check()
    except (GraphError, UsageError) as e:
        print(f"eicrecon-py: graph error:\n{e}", file=sys.stderr)
        return 2

    if "dump-graph" in opts.flags:
        print(app.to_dot())
        return 0
    if "check" in opts.flags:
        print(f"{len(app.nodes)} Python factories, {len(app.plugins)} C++ plugins")
        print("inputs expected from source or C++ plugins:", ", ".join(external))
        return 0

    # Same precedence as eicrecon
    for flag, mode in [("configs", "configs"), ("dumpconfigs", "dump-configs"),
                       ("benchmark", "benchmark"), ("list-factories", "list-factories")]:
        if flag in opts.flags:
            break
    else:
        mode = "run"
    plugin_paths = []
    if os.environ.get("EICrecon_MY"):
        plugin_paths.append(os.path.join(os.environ["EICrecon_MY"], "plugins"))
    print()
    return app.run(mode, load_config_file=opts.load_config_file,
                   dump_config_file=opts.dump_config_file or "jana.config",
                   plugin_paths=plugin_paths)


if __name__ == "__main__":
    sys.exit(main())
