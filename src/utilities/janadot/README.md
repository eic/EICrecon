# JANADOT Plugin

This plugin creates DOT (graphviz) files from JANA2 call graphs for visualization purposes.

## Overview

The janadot plugin records call stack information during event processing and generates DOT format files that can be processed by graphviz to create visual call graphs. It includes functionality to split large graphs by plugin or by user-defined groups for better organization and readability.

## Usage

To use the plugin, add it to your eicrecon command:

```bash
eicrecon -Pplugins=janadot sim_file.edm4hep.root
```

## Configuration Parameters

The plugin supports several configuration parameters:

- `janadot:output_file` (default: "jana.dot") - Output DOT filename
- `janadot:enable_splitting` (default: true) - Enable splitting graphs into multiple files
- `janadot:split_criteria` (default: "plugin") - Criteria for splitting graphs: plugin or groups

## Plugin-based Splitting

To use plugin-based splitting to group components by detector subsystem plugins:

```bash
eicrecon -Pplugins=janadot -Pjanadot:split_criteria=plugin sim_file.root
```

This generates:
- `jana.tracking.dot` - All tracking-related components
- `jana.ecal_barrel.dot` - ECAL barrel subsystem components
- `jana.hcal_endcap.dot` - HCAL endcap subsystem components
- `jana.dot` - Overall inter-plugin connection summary

## Group-based Splitting

To use user-defined group splitting based on the `.github/janadot.groups` file:

```bash
eicrecon -Pplugins=janadot -Pjanadot:split_criteria=groups sim_file.root
```

This generates:
- `jana.EcalEndcapN.dot` - Components in the EcalEndcapN group
- `jana.HcalBarrel.dot` - Components in the HcalBarrel group
- `jana.Tracking.dot` - Components in the Tracking group
- `jana.dot` - Overall inter-group connection summary

Groups can also be defined on the command line:

```bash
eicrecon -Pplugins=janadot \
  -Pjanadot:split_criteria=groups \
  -Pjanadot:group:MyGroup="Factory1:Tag1,Factory2:Tag2,color_blue" \
  sim_file.root
```

## Output Files

When splitting is disabled or graphs are small enough, a single DOT file is generated. When splitting is enabled and graphs are large, multiple files are created:

- `jana.plugin_name.dot` - Plugin-specific graphs (for plugin splitting)
- `jana.GroupName.dot` - Group-specific graphs (for groups splitting)
- `jana.dot` - Main graph showing inter-plugin/inter-group connections

## Generating Graphics

To convert DOT files to graphics:

```bash
# For a single file
dot -Tpdf jana.dot -o jana.pdf

# For plugin-based splits
dot -Tpdf jana.tracking.dot -o jana.tracking.pdf
dot -Tpdf jana.ecal_barrel.dot -o jana.ecal_barrel.pdf

# For group-based splits
dot -Tpdf jana.EcalEndcapN.dot -o jana.EcalEndcapN.pdf
dot -Tpdf jana.HcalBarrel.dot -o jana.HcalBarrel.pdf
```
