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
- `janadot:group:GroupName` - Define custom groups that override plugin-based assignment

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
