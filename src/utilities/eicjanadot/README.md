# EIC JANADOT Plugin

This plugin creates DOT (graphviz) files from JANA2 call graphs for visualization purposes.

## Overview

The eicjanadot plugin records call stack information during event processing and generates DOT format files that can be processed by graphviz to create visual call graphs. It includes functionality to split large graphs into multiple smaller graphs for better processing and readability.

## Usage

To use the plugin, add it to your eicrecon command:

```bash
eicrecon -Pplugins=eicjanadot sim_file.edm4hep.root
```

## Configuration Parameters

The plugin supports several configuration parameters:

- `eicjanadot:output_file` (default: "jana.dot") - Output DOT filename
- `eicjanadot:enable_splitting` (default: true) - Enable splitting large graphs into multiple files
- `eicjanadot:max_nodes_per_graph` (default: 50) - Maximum number of nodes per graph when splitting
- `eicjanadot:max_edges_per_graph` (default: 100) - Maximum number of edges per graph when splitting
- `eicjanadot:split_criteria` (default: "plugin") - Criteria for splitting graphs: size, components, type, plugin

## Plugin-based Splitting

The default splitting method groups components by detector subsystem plugins:

```bash
eicrecon -Pplugins=eicjanadot -Peicjanadot:split_criteria=plugin sim_file.root
```

This generates:
- `jana.tracking.dot` - All tracking-related components
- `jana.ecal_barrel.dot` - ECAL barrel subsystem components
- `jana.hcal_endcap.dot` - HCAL endcap subsystem components  
- `jana.dot` - Overall inter-plugin connection summary

## Output Files

When splitting is disabled or graphs are small enough, a single DOT file is generated. When splitting is enabled and graphs are large, multiple files are created:

- `jana_part001.dot`, `jana_part002.dot`, etc. - Individual graph parts (for size/components/type splitting)
- `jana.plugin_name.dot` - Plugin-specific graphs (for plugin splitting)
- `jana.dot` - Main graph showing inter-plugin connections (for plugin splitting)
- `jana_index.txt` - Index file explaining the split and how to process the files

## Generating Graphics

To convert DOT files to graphics:

```bash
# For a single file
dot -Tpdf jana.dot -o jana.pdf

# For split files (as shown in the index file)
dot -Tpdf jana_part001.dot -o jana_part001.pdf
dot -Tpdf jana_part002.dot -o jana_part002.pdf
# ... etc

# For plugin-based splits
dot -Tpdf jana.tracking.dot -o jana.tracking.pdf
dot -Tpdf jana.ecal_barrel.dot -o jana.ecal_barrel.pdf
```

## Graph Splitting Methods

### Plugin-based Splitting (default)
Groups components by detector subsystem, providing both detailed subsystem views and high-level architectural overview.

### Size-based Splitting
Nodes are grouped to keep within the specified limits of nodes and edges per graph.

### Component-based Splitting
Uses connected components analysis to group nodes that are connected by call relationships.

### Type-based Splitting
Groups nodes by their type (Processor, Factory, Source, etc.).
