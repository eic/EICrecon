# JANADOT Plugin

This plugin creates DOT (graphviz) files from JANA2 call graphs for visualization purposes.

## Overview

The janadot plugin records call stack information during event processing and generates DOT format files that can be processed by graphviz to create visual call graphs. It includes functionality to split large graphs into multiple smaller graphs for better processing and readability.

## Usage

To use the plugin, add it to your eicrecon command:

```bash
eicrecon -Pplugins=janadot sim_file.edm4hep.root
```

## Configuration Parameters

The plugin supports several configuration parameters:

- `janadot:output_file` (default: "jana.dot") - Output DOT filename
- `janadot:enable_splitting` (default: true) - Enable splitting large graphs into multiple files
- `janadot:max_nodes_per_graph` (default: 50) - Maximum number of nodes per graph when splitting
- `janadot:max_edges_per_graph` (default: 100) - Maximum number of edges per graph when splitting
- `janadot:split_criteria` (default: "size") - Criteria for splitting graphs: size, components, type

## Output Files

When splitting is disabled or graphs are small enough, a single DOT file is generated. When splitting is enabled and graphs are large, multiple files are created:

- `jana_part001.dot`, `jana_part002.dot`, etc. - Individual graph parts
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
```

## Graph Splitting Methods

### Size-based Splitting
Nodes are grouped to keep within the specified limits of nodes and edges per graph.

### Component-based Splitting
Uses connected components analysis to group nodes that are connected by call relationships.

### Type-based Splitting
Groups nodes by their type (Processor, Factory, Source, etc.).
