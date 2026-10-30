# JANADOT Plugin

This plugin creates DOT (graphviz) files from JANA2 call graphs for visualization purposes.

## Overview

The janadot plugin records call stack information during event processing and generates DOT format files that can be processed by graphviz to create visual call graphs. It automatically splits graphs by plugin/detector subsystem for better organization and readability, with support for user-defined group overrides.

## Usage

To use the plugin, add it to your eicrecon command:

```bash
eicrecon -Pplugins=janadot sim_file.edm4hep.root
```

## Configuration Parameters

The plugin supports the following configuration parameters:

- `janadot:output_file` (default: "jana.dot") - Output DOT filename
- `janadot:enable_splitting` (default: true) - Enable splitting graphs into multiple files by plugin
- `janadot:group:<GroupName>="ObjectType:Tag,...,color_<color>"` - Define a custom group
  named `<GroupName>` (your choice) that overrides plugin-based assignment for the listed
  factories; see [Custom Group Overrides](#custom-group-overrides) below

## Plugin-based Splitting (Default)

By default, the plugin groups components by detector subsystem plugins:

```bash
eicrecon -Pplugins=janadot sim_file.edm4hep.root
```

This generates:
- `jana.tracking.dot` - All tracking-related components
- `jana.ecal_barrel.dot` - ECAL barrel subsystem components
- `jana.hcal_endcap.dot` - HCAL endcap subsystem components
- `jana.dot` - Overall inter-plugin connection summary

## Custom Group Overrides

You can override the default plugin-based grouping for specific factories on the command line.
Each entry is `ObjectType:Tag`, where the object type is the factory's primary output type
(`GetObjectName()`), not the C++ factory class name:

```bash
# Override default grouping for specific factories
eicrecon -Pplugins=janadot \
  -Pjanadot:group:MyCustomGroup="edm4eic::Cluster:EcalBarrelClusters,color_blue" \
  sim_file.edm4hep.root
```

Group syntax: `ObjectType:Tag,AnotherObjectType:AnotherTag,color_<color>` (e.g. `color_blue`)

## Output Files

When splitting is enabled (default), multiple files are created:

- `jana.plugin_name.dot` - Plugin-specific graphs (or custom group name if overridden)
- `jana.dot` - Main graph showing inter-plugin/inter-group connections

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
dot -Tpdf jana.dot -o jana.pdf

# For plugin-based splits
dot -Tpdf jana.tracking.dot -o jana.tracking.pdf
dot -Tpdf jana.ecal_barrel.dot -o jana.ecal_barrel.pdf

# For custom groups
dot -Tpdf jana.MyCustomGroup.dot -o jana.MyCustomGroup.pdf
```

The overview graph (`jana.dot`) links to each plugin/group's file via an SVG URL, so generate
those as SVG rather than PDF if you want the links to resolve when opened (e.g. in a browser):

```bash
dot -Tsvg jana.dot -o jana.svg
dot -Tsvg jana.tracking.dot -o jana.tracking.svg
dot -Tsvg jana.MyCustomGroup.dot -o jana.MyCustomGroup.svg
```
