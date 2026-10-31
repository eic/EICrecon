# Callgraph Plugin

This plugin creates DOT (graphviz) files from JANA2 call graphs for visualization purposes.
It is based on JANA2's `janadot` plugin, but is a separate plugin with its own name and
parameters, so `-Pplugins=janadot` still loads JANA2's version.

## Overview

The callgraph plugin records call stack information during event processing and generates DOT format files that can be processed by graphviz to create visual call graphs. It automatically splits graphs by plugin/detector subsystem for better organization and readability, with support for user-defined group overrides.

## Usage

To use the plugin, add it to your eicrecon command:

```bash
eicrecon -Pplugins=callgraph sim_file.edm4hep.root
```

## Configuration Parameters

The plugin supports the following configuration parameters:

- `callgraph:output_file` (default: "callgraph.dot") - Output DOT filename
- `callgraph:enable_splitting` (default: true) - Enable splitting graphs into multiple files by plugin
- `callgraph:LogLevel` (default: `eicrecon:LogLevel`) - Level of the plugin's messages
- `callgraph:group:<GroupName>="Tag,...,color_<color>"` - Define a custom group
  named `<GroupName>` (your choice) that overrides plugin-based assignment for the listed
  factories; see [Custom Group Overrides](#custom-group-overrides) below

## Plugin-based Splitting (Default)

By default, the plugin groups components by detector subsystem plugins:

```bash
eicrecon -Pplugins=callgraph sim_file.edm4hep.root
```

This generates:
- `callgraph.tracking.dot` - All tracking-related components
- `callgraph.BEMC.dot` - Barrel ECAL components
- `callgraph.EHCAL.dot` - Backward HCAL components
- `callgraph.dot` - Overall inter-plugin connection summary

## Custom Group Overrides

You can override the default plugin-based grouping for specific factories on the command line.
Each entry is a factory's tag: the name on its box in the graphs, which is the name of its output
collection (the first one, for a factory with several):

```bash
# Override default grouping for specific factories
eicrecon -Pplugins=callgraph \
  -Pcallgraph:group:MyCustomGroup="EcalBarrelClustersWithoutShapes,EcalBarrelClusters,color_blue" \
  sim_file.edm4hep.root
```

Group syntax: `Tag,AnotherTag,color_<color>` (e.g. `color_blue`). An entry that matches nothing
in the call graph is ignored, with a warning at the end of the job.

An entry can also be written as `ObjectType:Tag`, where the object type is the factory's
`GetObjectName()`. Only factories based on `JFactoryT` have an object type, and they must be
written this way; for all other factories, including every `JOmniFactory`, it is ignored.

## Output Files

When splitting is enabled (default), multiple files are created:

- `callgraph.plugin_name.dot` - Plugin-specific graphs (or custom group name if overridden)
- `callgraph.dot` - Main graph showing inter-plugin/inter-group connections

## Reading the Graphs

Each graph has a legend under its title that defines the quantities in its labels:

- An arrow `A -> B` means that `A` requested data from `B`. Its label shows the number of
  requests (*calls*, including those served from cache) and the *edge time*: the time `A`
  waited for that data, summed over all calls. A factory runs once per event, for the first
  request; that request's time includes the factory's self time and all of its outgoing edges,
  while later requests are served from cache and add almost nothing.
- A node's label shows its *self time*: the time spent in its own code, not waiting for its
  inputs. It equals the sum of its incoming edge times minus the sum of its outgoing edge
  times. Event processors make the top-level requests; their own time isn't measured (shown
  as 0), and the edges leaving them add up to the total.
- All percentages, in all graphs, are relative to the same total `T`: the self time of all
  nodes, i.e. the time spent in all factories, summed over all events (on all threads).
- In the overview graph, a plugin's time is the sum of its factories' self times, which also
  equals the sum of its incoming minus its outgoing edge times, as requests between factories
  in the same plugin cancel out. A plugin graph doesn't draw requests to or from other
  plugins, but they still count in these sums.

## Generating Graphics

To convert DOT files to graphics:

```bash
# For a single file
dot -Tpdf callgraph.dot -o callgraph.pdf

# For plugin-based splits
dot -Tpdf callgraph.tracking.dot -o callgraph.tracking.pdf
dot -Tpdf callgraph.BEMC.dot -o callgraph.BEMC.pdf

# For custom groups
dot -Tpdf callgraph.MyCustomGroup.dot -o callgraph.MyCustomGroup.pdf
```

The overview graph (`callgraph.dot`) links to each plugin/group's file via an SVG URL, so generate
those as SVG rather than PDF if you want the links to resolve when opened (e.g. in a browser):

```bash
dot -Tsvg callgraph.dot -o callgraph.svg
dot -Tsvg callgraph.tracking.dot -o callgraph.tracking.svg
dot -Tsvg callgraph.MyCustomGroup.dot -o callgraph.MyCustomGroup.svg
```
