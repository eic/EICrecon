// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, EICrecon contributors

#include "JEventProcessorJANADOT.h"

#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/JEvent.h>
#include <JANA/Utils/JCallGraphRecorder.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <chrono>

void JEventProcessorJANADOT::Init() {
  // Get parameter manager
  auto params = GetApplication()->GetJParameterManager();

  // Set default parameter values and register them
  output_filename = "jana.dot";
  params->SetDefaultParameter("janadot:output_file", output_filename, "Output DOT filename");

  enable_splitting = true;
  params->SetDefaultParameter("janadot:enable_splitting", enable_splitting,
                              "Enable splitting large graphs into multiple files");

  max_nodes_per_graph = 50;
  params->SetDefaultParameter("janadot:max_nodes_per_graph", max_nodes_per_graph,
                              "Maximum number of nodes per graph when splitting");

  max_edges_per_graph = 100;
  params->SetDefaultParameter("janadot:max_edges_per_graph", max_edges_per_graph,
                              "Maximum number of edges per graph when splitting");

  split_criteria = "size";
  params->SetDefaultParameter("janadot:split_criteria", split_criteria,
                              "Criteria for splitting graphs: size, components, type");
}

void JEventProcessorJANADOT::Process(const std::shared_ptr<const JEvent>& event) {
  // Get the call stack for this event and add the results to our stats
  auto stack = event->GetJCallGraphRecorder()->GetCallGraph();

  // Lock mutex in case we are running with multiple threads
  std::lock_guard<std::mutex> lck(mutex);

  // Loop over the call stack elements and add in the values
  for (unsigned int i = 0; i < stack.size(); i++) {

    // Keep track of total time each factory spent waiting and being waited on
    std::string nametag1 = MakeNametag(stack[i].caller_name, stack[i].caller_tag);
    std::string nametag2 = MakeNametag(stack[i].callee_name, stack[i].callee_tag);

    FactoryCallStats& fcallstats1 = factory_stats[nametag1];
    FactoryCallStats& fcallstats2 = factory_stats[nametag2];

    // Determine node types
    fcallstats1.type = GetNodeType(stack[i].caller_name, stack[i].caller_tag);
    fcallstats2.type = GetNodeType(stack[i].callee_name, stack[i].callee_tag);

    auto delta_t_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stack[i].end_time -
                                                                            stack[i].start_time)
                          .count();
    fcallstats1.time_waiting += delta_t_ms;
    fcallstats2.time_waited_on += delta_t_ms;

    // Get pointer to CallStats object representing this calling pair
    CallLink link;
    link.caller_name = stack[i].caller_name;
    link.caller_tag  = stack[i].caller_tag;
    link.callee_name = stack[i].callee_name;
    link.callee_tag  = stack[i].callee_tag;
    CallStats& stats =
        call_links[link]; // get pointer to stats object or create if it doesn't exist

    switch (stack[i].data_source) {
    case JCallGraphRecorder::DATA_NOT_AVAILABLE:
      stats.Ndata_not_available++;
      stats.data_not_available_ms += delta_t_ms;
      break;
    case JCallGraphRecorder::DATA_FROM_CACHE:
      fcallstats2.Nfrom_cache++;
      stats.Nfrom_cache++;
      stats.from_cache_ms += delta_t_ms;
      break;
    case JCallGraphRecorder::DATA_FROM_SOURCE:
      fcallstats2.Nfrom_source++;
      stats.Nfrom_source++;
      stats.from_source_ms += delta_t_ms;
      break;
    case JCallGraphRecorder::DATA_FROM_FACTORY:
      fcallstats2.Nfrom_factory++;
      stats.Nfrom_factory++;
      stats.from_factory_ms += delta_t_ms;
      break;
    }
  }
}

void JEventProcessorJANADOT::Finish() { WriteDotFile(); }

void JEventProcessorJANADOT::WriteDotFile() {
  int total_nodes, total_edges;
  AnalyzeGraph(total_nodes, total_edges);

  std::cout << "Graph analysis: " << total_nodes << " nodes, " << total_edges << " edges"
            << std::endl;

  if (enable_splitting && ShouldSplitGraph(total_nodes, total_edges)) {
    std::cout << "Graph is large, splitting into multiple files..." << std::endl;
    WriteSplitDotFiles();
  } else {
    WriteSingleDotFile(output_filename);
    std::cout << std::endl;
    std::cout << "Factory calling information written to \"" << output_filename
              << "\". To create a graphic" << std::endl;
    std::cout << "from this, use the dot program. For example, to make a PDF file do the following:"
              << std::endl;
    std::cout << std::endl;
    std::cout << "   dot -Tpdf " << output_filename << " -o jana.pdf" << std::endl;
    std::cout << std::endl;
    std::cout << "This should give you a file named \"jana.pdf\"." << std::endl;
    std::cout << std::endl;
  }
}

void JEventProcessorJANADOT::WriteSingleDotFile(const std::string& filename) {
  std::cout << "Opening output file \"" << filename << "\"" << std::endl;

  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing!" << std::endl;
    return;
  }

  // Calculate total time for percentages
  double total_ms = 0.0;
  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    // Only count top-level callers (those not called by others)
    bool is_top_level = true;
    for (auto& [other_link, other_stats] : call_links) {
      std::string other_callee = MakeNametag(other_link.callee_name, other_link.callee_tag);
      if (other_callee == caller) {
        is_top_level = false;
        break;
      }
    }
    if (is_top_level) {
      total_ms += stats.from_factory_ms + stats.from_source_ms + stats.from_cache_ms +
                  stats.data_not_available_ms;
    }
  }
  if (total_ms == 0.0)
    total_ms = 1.0;

  // Write DOT file header
  ofs << "digraph G {" << std::endl;
  ofs << "  rankdir=TB;" << std::endl;
  ofs << "  node [fontname=\"Arial\", fontsize=10];" << std::endl;
  ofs << "  edge [fontname=\"Arial\", fontsize=8];" << std::endl;
  ofs << std::endl;

  // Write nodes
  for (auto& [nametag, fstats] : factory_stats) {
    double time_in_factory = fstats.time_waited_on - fstats.time_waiting;
    double percent         = 100.0 * time_in_factory / total_ms;

    std::string color = GetNodeColor(fstats.type);
    std::string shape = GetNodeShape(fstats.type);

    ofs << "  \"" << nametag << "\" [";
    ofs << "fillcolor=" << color << ", ";
    ofs << "style=filled, ";
    ofs << "shape=" << shape << ", ";
    ofs << "label=\"" << nametag << "\\n";
    ofs << MakeTimeString(time_in_factory) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << std::endl;

  // Write edges
  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee = MakeNametag(link.callee_name, link.callee_tag);

    unsigned int total_calls =
        stats.Nfrom_cache + stats.Nfrom_source + stats.Nfrom_factory + stats.Ndata_not_available;
    double total_time = stats.from_cache_ms + stats.from_source_ms + stats.from_factory_ms +
                        stats.data_not_available_ms;
    double percent = 100.0 * total_time / total_ms;

    ofs << "  \"" << caller << "\" -> \"" << callee << "\" [";
    ofs << "label=\"" << total_calls << " calls\\n";
    ofs << MakeTimeString(total_time) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << "}" << std::endl;
  ofs.close();
}

void JEventProcessorJANADOT::WriteSplitDotFiles() {
  std::vector<std::set<std::string>> node_groups;

  if (split_criteria == "components") {
    node_groups = SplitGraphByConnectedComponents();
  } else if (split_criteria == "type") {
    node_groups = SplitGraphByType();
  } else { // default to "size"
    node_groups = SplitGraphBySize();
  }

  std::cout << "Splitting graph into " << node_groups.size() << " subgraphs" << std::endl;

  for (size_t i = 0; i < node_groups.size(); i++) {
    // Create filename for this subgraph
    std::string base_filename = output_filename;
    size_t dot_pos            = base_filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
      base_filename = base_filename.substr(0, dot_pos);
    }

    std::stringstream ss;
    ss << base_filename << "_part" << std::setfill('0') << std::setw(3) << (i + 1) << ".dot";
    std::string filename = ss.str();

    WriteSplitDotFile(filename, node_groups[i]);
  }

  // Write an index file explaining the split
  WriteIndexFile(node_groups.size());
}

void JEventProcessorJANADOT::WriteSplitDotFile(const std::string& filename,
                                               const std::set<std::string>& nodes) {
  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing!" << std::endl;
    return;
  }

  // Calculate total time for this subgraph
  double total_ms = 0.0;
  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    if (nodes.find(caller) != nodes.end()) {
      total_ms += stats.from_factory_ms + stats.from_source_ms + stats.from_cache_ms +
                  stats.data_not_available_ms;
    }
  }
  if (total_ms == 0.0)
    total_ms = 1.0;

  // Write DOT file header
  ofs << "digraph G {" << std::endl;
  ofs << "  rankdir=TB;" << std::endl;
  ofs << "  node [fontname=\"Arial\", fontsize=10];" << std::endl;
  ofs << "  edge [fontname=\"Arial\", fontsize=8];" << std::endl;
  ofs << "  label=\"EICrecon Call Graph (Part " << filename.substr(filename.find("part")) << ")\";"
      << std::endl;
  ofs << "  labelloc=\"t\";" << std::endl;
  ofs << std::endl;

  // Write nodes (only those in this group)
  for (const std::string& nametag : nodes) {
    auto fstats_it = factory_stats.find(nametag);
    if (fstats_it == factory_stats.end())
      continue;

    const FactoryCallStats& fstats = fstats_it->second;
    double time_in_factory         = fstats.time_waited_on - fstats.time_waiting;
    double percent                 = 100.0 * time_in_factory / total_ms;

    std::string color = GetNodeColor(fstats.type);
    std::string shape = GetNodeShape(fstats.type);

    ofs << "  \"" << nametag << "\" [";
    ofs << "fillcolor=" << color << ", ";
    ofs << "style=filled, ";
    ofs << "shape=" << shape << ", ";
    ofs << "label=\"" << nametag << "\\n";
    ofs << MakeTimeString(time_in_factory) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << std::endl;

  // Write edges (only those within this group)
  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee = MakeNametag(link.callee_name, link.callee_tag);

    // Only include edges where both nodes are in this group
    if (nodes.find(caller) == nodes.end() || nodes.find(callee) == nodes.end()) {
      continue;
    }

    unsigned int total_calls =
        stats.Nfrom_cache + stats.Nfrom_source + stats.Nfrom_factory + stats.Ndata_not_available;
    double total_time = stats.from_cache_ms + stats.from_source_ms + stats.from_factory_ms +
                        stats.data_not_available_ms;
    double percent = 100.0 * total_time / total_ms;

    ofs << "  \"" << caller << "\" -> \"" << callee << "\" [";
    ofs << "label=\"" << total_calls << " calls\\n";
    ofs << MakeTimeString(total_time) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << "}" << std::endl;
  ofs.close();
}

void JEventProcessorJANADOT::WriteIndexFile(int num_parts) {
  std::string base_filename = output_filename;
  size_t dot_pos            = base_filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    base_filename = base_filename.substr(0, dot_pos);
  }

  std::string index_filename = base_filename + "_index.txt";
  std::ofstream ofs(index_filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open index file " << index_filename << " for writing!"
              << std::endl;
    return;
  }

  ofs << "EICrecon Call Graph Split Information" << std::endl;
  ofs << "=====================================" << std::endl;
  ofs << std::endl;
  ofs << "The call graph was too large for efficient processing by graphviz," << std::endl;
  ofs << "so it has been split into " << num_parts << " separate DOT files:" << std::endl;
  ofs << std::endl;

  for (int i = 1; i <= num_parts; i++) {
    std::stringstream ss;
    ss << base_filename << "_part" << std::setfill('0') << std::setw(3) << i << ".dot";
    ofs << "  " << ss.str() << std::endl;
  }

  ofs << std::endl;
  ofs << "To generate PDF files from each part:" << std::endl;
  ofs << std::endl;

  for (int i = 1; i <= num_parts; i++) {
    std::stringstream ss_dot, ss_pdf;
    ss_dot << base_filename << "_part" << std::setfill('0') << std::setw(3) << i << ".dot";
    ss_pdf << base_filename << "_part" << std::setfill('0') << std::setw(3) << i << ".pdf";
    ofs << "  dot -Tpdf " << ss_dot.str() << " -o " << ss_pdf.str() << std::endl;
  }

  ofs << std::endl;
  ofs << "Configuration used:" << std::endl;
  ofs << "  Split criteria: " << split_criteria << std::endl;
  ofs << "  Max nodes per graph: " << max_nodes_per_graph << std::endl;
  ofs << "  Max edges per graph: " << max_edges_per_graph << std::endl;

  ofs.close();

  std::cout << std::endl;
  std::cout << "Factory calling information written to " << num_parts << " files." << std::endl;
  std::cout << "See \"" << index_filename << "\" for details on processing the files." << std::endl;
  std::cout << std::endl;
}

std::string JEventProcessorJANADOT::MakeTimeString(double time_in_ms) {
  double order = log10(std::abs(time_in_ms) + 1e-9); // avoid log(0)
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  if (order < 0) {
    ss << time_in_ms * 1000.0 << " us";
  } else if (order <= 2.0) {
    ss << time_in_ms << " ms";
  } else {
    ss << time_in_ms / 1000.0 << " s";
  }
  return ss.str();
}

std::string JEventProcessorJANADOT::MakeNametag(const std::string& name, const std::string& tag) {
  std::string nametag = name;
  if (tag.size() > 0)
    nametag += ":" + tag;
  return nametag;
}

JEventProcessorJANADOT::node_type JEventProcessorJANADOT::GetNodeType(const std::string& name,
                                                                      const std::string& tag) {
  // Simple heuristics to determine node type based on name patterns
  if (name.find("Processor") != std::string::npos || name.find("PROCESSOR") != std::string::npos) {
    return kProcessor;
  } else if (name.find("Source") != std::string::npos || name.find("SOURCE") != std::string::npos) {
    return kSource;
  } else if (name.find("Cache") != std::string::npos || name.find("CACHE") != std::string::npos) {
    return kCache;
  } else if (!tag.empty() || name.find("Factory") != std::string::npos ||
             name.find("edm4") != std::string::npos) {
    return kFactory;
  }
  return kDefault;
}

std::string JEventProcessorJANADOT::GetNodeColor(node_type type) {
  switch (type) {
  case kProcessor:
    return "lightgreen";
  case kFactory:
    return "lightblue";
  case kCache:
    return "yellow";
  case kSource:
    return "lightcoral";
  default:
    return "white";
  }
}

std::string JEventProcessorJANADOT::GetNodeShape(node_type type) {
  switch (type) {
  case kProcessor:
    return "ellipse";
  case kFactory:
    return "box";
  case kCache:
    return "diamond";
  case kSource:
    return "trapezium";
  default:
    return "ellipse";
  }
}

void JEventProcessorJANADOT::AnalyzeGraph(int& total_nodes, int& total_edges) {
  total_nodes = factory_stats.size();
  total_edges = call_links.size();
}

bool JEventProcessorJANADOT::ShouldSplitGraph(int total_nodes, int total_edges) {
  return (total_nodes > max_nodes_per_graph) || (total_edges > max_edges_per_graph);
}

std::vector<std::set<std::string>> JEventProcessorJANADOT::SplitGraphBySize() {
  std::vector<std::set<std::string>> groups;
  std::set<std::string> current_group;

  int current_nodes = 0;
  int current_edges = 0;

  // Simple greedy algorithm: add nodes to current group until limits are reached
  for (auto& [nametag, fstats] : factory_stats) {
    // Count edges involving this node
    int node_edges = 0;
    for (auto& [link, stats] : call_links) {
      std::string caller = MakeNametag(link.caller_name, link.caller_tag);
      std::string callee = MakeNametag(link.callee_name, link.callee_tag);
      if (caller == nametag || callee == nametag) {
        // Only count edge if both nodes are in current group (or this is the first node)
        if (current_group.empty() || current_group.find(caller) != current_group.end() ||
            current_group.find(callee) != current_group.end()) {
          node_edges++;
        }
      }
    }

    // Check if adding this node would exceed limits
    if (current_nodes > 0 && (current_nodes + 1 > max_nodes_per_graph ||
                              current_edges + node_edges > max_edges_per_graph)) {
      groups.push_back(current_group);
      current_group.clear();
      current_nodes = 0;
      current_edges = 0;
    }

    current_group.insert(nametag);
    current_nodes++;
    current_edges += node_edges;
  }

  if (!current_group.empty()) {
    groups.push_back(current_group);
  }

  // If we only have one group, return it as-is (even if it's large)
  if (groups.empty()) {
    std::set<std::string> all_nodes;
    for (auto& [nametag, fstats] : factory_stats) {
      all_nodes.insert(nametag);
    }
    groups.push_back(all_nodes);
  }

  return groups;
}

std::vector<std::set<std::string>> JEventProcessorJANADOT::SplitGraphByConnectedComponents() {
  // Implementation of connected components finding using Union-Find
  std::map<std::string, std::string> parent;

  // Initialize each node as its own parent
  for (auto& [nametag, fstats] : factory_stats) {
    parent[nametag] = nametag;
  }

  // Union-Find helper functions
  std::function<std::string(const std::string&)> find = [&](const std::string& x) -> std::string {
    if (parent[x] != x) {
      parent[x] = find(parent[x]);
    }
    return parent[x];
  };

  auto unite = [&](const std::string& x, const std::string& y) {
    std::string px = find(x);
    std::string py = find(y);
    if (px != py) {
      parent[px] = py;
    }
  };

  // Connect nodes that have edges between them
  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee = MakeNametag(link.callee_name, link.callee_tag);
    unite(caller, callee);
  }

  // Group nodes by their root parent
  std::map<std::string, std::set<std::string>> components;
  for (auto& [nametag, fstats] : factory_stats) {
    components[find(nametag)].insert(nametag);
  }

  // Convert to vector of sets
  std::vector<std::set<std::string>> groups;
  for (auto& [root, component] : components) {
    groups.push_back(component);
  }

  return groups;
}

std::vector<std::set<std::string>> JEventProcessorJANADOT::SplitGraphByType() {
  std::map<node_type, std::set<std::string>> type_groups;

  // Group nodes by their type
  for (auto& [nametag, fstats] : factory_stats) {
    type_groups[fstats.type].insert(nametag);
  }

  // Convert to vector of sets
  std::vector<std::set<std::string>> groups;
  for (auto& [type, group] : type_groups) {
    if (!group.empty()) {
      groups.push_back(group);
    }
  }

  return groups;
}
