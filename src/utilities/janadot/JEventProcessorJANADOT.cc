// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, EICrecon contributors

#include "JEventProcessorJANADOT.h"

#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <JANA/JFactory.h>
#include <JANA/JFactorySet.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JCallGraphRecorder.h>
#include <ctype.h>
#include <stddef.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>

void JEventProcessorJANADOT::Init() {
  // Get parameter manager
  auto params = GetApplication()->GetJParameterManager();

  // Set default parameter values and register them
  output_filename = "jana.dot";
  params->SetDefaultParameter("janadot:output_file", output_filename, "Output DOT filename");

  enable_splitting = true;
  params->SetDefaultParameter("janadot:enable_splitting", enable_splitting,
                              "Enable splitting graphs into multiple files by plugin");

  // Check for janadot:group parameters (command line group definitions)
  // These override the default plugin-based group assignment
  std::map<std::string, std::string> parameter_keys;
  params->FilterParameters(parameter_keys, "janadot:group:");
  for (const auto& [group_name, group_definition] : parameter_keys) {
    // Parse command line group definition similar to file format
    std::vector<std::string> factories;
    std::string color = "lightblue"; // default color

    std::stringstream ss(group_definition);
    std::string item;
    while (std::getline(ss, item, ',')) {
      if (item.find("color_") == 0) {
        color = item.substr(6); // remove "color_" prefix
      } else if (!item.empty()) {
        factories.push_back(item);
      }
    }

    user_groups[group_name]       = factories;
    user_group_colors[group_name] = color;

    // Build nametag to group mapping (overrides plugin-based assignment)
    for (const auto& factory : factories) {
      nametag_to_group[factory] = group_name;
    }
  }
}

void JEventProcessorJANADOT::Process(const std::shared_ptr<const JEvent>& event) {
  // Get the call stack for this event and add the results to our stats
  auto stack = event->GetJCallGraphRecorder()->GetCallGraph();

  // Lock mutex in case we are running with multiple threads
  std::lock_guard<std::mutex> lck(mutex);

  // Build mapping of nametags to plugin names (only do this once per execution)
  static bool factory_mapping_built = false;
  if (!factory_mapping_built) {
    auto factories = event->GetFactorySet()->GetAllFactories();
    for (auto* factory : factories) {
      std::string nametag     = MakeNametag(factory->GetObjectName(), factory->GetTag());
      std::string plugin_name = factory->GetPluginName();

      // If plugin name is empty, try to use a reasonable default
      if (plugin_name.empty()) {
        plugin_name = "core";
      }

      nametag_to_plugin[nametag] = plugin_name;
    }
    factory_mapping_built = true;
  }

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
  if (enable_splitting) {
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
  // Always use plugin-based splitting with optional user group overrides
  std::map<std::string, std::set<std::string>> groups = SplitGraphByPlugin();
  WritePluginGraphs(groups);
  WriteOverallDotFile(groups);
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
  std::string nametag = tag;
  if (tag.size() > 0 && name.size() > 0) {
    nametag += " (" + name + ")";
  } else if (name.size() > 0) {
    nametag = name;
  }
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

void JEventProcessorJANADOT::WritePluginGraphs(
    const std::map<std::string, std::set<std::string>>& plugin_groups) {
  std::cout << "Splitting graph into " << plugin_groups.size() << " plugin-based subgraphs"
            << std::endl;

  // Write individual plugin dot files
  for (auto& [plugin_name, nodes] : plugin_groups) {
    WritePluginDotFile(plugin_name, nodes);
  }

  std::cout << std::endl;
  std::cout << "Factory calling information written to " << plugin_groups.size()
            << " plugin files plus overall summary." << std::endl;
  std::cout << "Plugin files use period-separated naming (e.g., jana.tracking.dot)." << std::endl;
  std::cout << "Overall summary in \"" << output_filename << "\" shows inter-plugin connections."
            << std::endl;
  std::cout << std::endl;
}

void JEventProcessorJANADOT::WritePluginDotFile(const std::string& plugin_name,
                                                const std::set<std::string>& nodes) {
  // Create filename using period-separated plugin name
  std::string base_filename = output_filename;
  size_t dot_pos            = base_filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    base_filename = base_filename.substr(0, dot_pos);
  }

  std::string filename = base_filename + "." + plugin_name + ".dot";

  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing!" << std::endl;
    return;
  }

  // Calculate total time for this plugin
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
  ofs << "  label=\"EICrecon Call Graph - " << plugin_name << " Plugin\";" << std::endl;
  ofs << "  labelloc=\"t\";" << std::endl;
  ofs << std::endl;

  // Write nodes (only those in this plugin)
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

  // Write edges (only those within this plugin)
  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee = MakeNametag(link.callee_name, link.callee_tag);

    // Only include edges where both nodes are in this plugin
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

void JEventProcessorJANADOT::WriteOverallDotFile(
    const std::map<std::string, std::set<std::string>>& plugin_groups) {
  std::ofstream ofs(output_filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << output_filename << " for writing!" << std::endl;
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
  ofs << "  node [fontname=\"Arial\", fontsize=12];" << std::endl;
  ofs << "  edge [fontname=\"Arial\", fontsize=10];" << std::endl;
  ofs << "  label=\"EICrecon Overall Call Graph - Inter-Plugin Connections\";" << std::endl;
  ofs << "  labelloc=\"t\";" << std::endl;
  ofs << std::endl;

  // Create plugin nodes (aggregate statistics per plugin)
  std::map<std::string, double> plugin_times;
  std::map<std::string, int> plugin_node_counts;

  for (auto& [plugin_name, nodes] : plugin_groups) {
    double plugin_time = 0.0;
    int node_count     = 0;

    for (const std::string& nametag : nodes) {
      auto fstats_it = factory_stats.find(nametag);
      if (fstats_it != factory_stats.end()) {
        const FactoryCallStats& fstats = fstats_it->second;
        plugin_time += fstats.time_waited_on - fstats.time_waiting;
        node_count++;
      }
    }

    plugin_times[plugin_name]       = plugin_time;
    plugin_node_counts[plugin_name] = node_count;

    double percent = 100.0 * plugin_time / total_ms;

    ofs << "  \"" << plugin_name << "\" [";
    ofs << "fillcolor=lightsteelblue, ";
    ofs << "style=filled, ";
    ofs << "shape=box, ";
    ofs << "label=\"" << plugin_name << "\\n";
    ofs << node_count << " components\\n";
    ofs << MakeTimeString(plugin_time) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << std::endl;

  // Create edges between plugins (when there are calls between different plugins)
  std::map<std::pair<std::string, std::string>, std::pair<int, double>> inter_plugin_calls;

  for (auto& [link, stats] : call_links) {
    std::string caller = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee = MakeNametag(link.callee_name, link.callee_tag);

    std::string caller_plugin = ExtractPluginName(caller);
    std::string callee_plugin = ExtractPluginName(callee);

    // Only show inter-plugin connections
    if (caller_plugin != callee_plugin && !caller_plugin.empty() && !callee_plugin.empty()) {
      auto key = std::make_pair(caller_plugin, callee_plugin);

      unsigned int total_calls =
          stats.Nfrom_cache + stats.Nfrom_source + stats.Nfrom_factory + stats.Ndata_not_available;
      double total_time = stats.from_cache_ms + stats.from_source_ms + stats.from_factory_ms +
                          stats.data_not_available_ms;

      inter_plugin_calls[key].first += total_calls;
      inter_plugin_calls[key].second += total_time;
    }
  }

  // Write inter-plugin edges
  for (auto& [plugins, call_data] : inter_plugin_calls) {
    int total_calls   = call_data.first;
    double total_time = call_data.second;
    double percent    = 100.0 * total_time / total_ms;

    ofs << "  \"" << plugins.first << "\" -> \"" << plugins.second << "\" [";
    ofs << "label=\"" << total_calls << " calls\\n";
    ofs << MakeTimeString(total_time) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\", ";
    // Scale penwidth linearly from 1 (0%) to 8 (100%)
    double penwidth = 1.0 + (percent / 100.0) * 7.0;
    ofs << "penwidth=" << std::fixed << std::setprecision(1) << penwidth;
    ofs << "];" << std::endl;
  }

  ofs << "}" << std::endl;
  ofs.close();
}

std::map<std::string, std::set<std::string>> JEventProcessorJANADOT::SplitGraphByPlugin() {
  std::map<std::string, std::set<std::string>> plugin_groups;

  // Group nodes by their actual plugin (from factory information)
  // with optional user group overrides
  for (auto& [nametag, fstats] : factory_stats) {
    std::string group_name;

    // Check if this factory has a user-defined group override
    auto override_it = nametag_to_group.find(nametag);
    if (override_it != nametag_to_group.end()) {
      // Use user-defined group
      group_name = override_it->second;
    } else {
      // Use plugin-based grouping
      auto it = nametag_to_plugin.find(nametag);
      if (it != nametag_to_plugin.end()) {
        group_name = it->second;
      } else {
        // Fall back to heuristic extraction for items not in factory set
        group_name = ExtractPluginName(nametag);
      }
    }

    plugin_groups[group_name].insert(nametag);
  }

  return plugin_groups;
}

std::string JEventProcessorJANADOT::ExtractPluginName(const std::string& nametag) {
  // Try to extract meaningful plugin names from component names

  // Common EIC patterns
  if (nametag.find("Ecal") != std::string::npos) {
    if (nametag.find("Barrel") != std::string::npos)
      return "ecal_barrel";
    if (nametag.find("Endcap") != std::string::npos)
      return "ecal_endcap";
    if (nametag.find("Insert") != std::string::npos)
      return "ecal_insert";
    if (nametag.find("LumiSpec") != std::string::npos)
      return "ecal_lumispec";
    return "ecal";
  }

  if (nametag.find("Hcal") != std::string::npos) {
    if (nametag.find("Barrel") != std::string::npos)
      return "hcal_barrel";
    if (nametag.find("Endcap") != std::string::npos)
      return "hcal_endcap";
    if (nametag.find("Insert") != std::string::npos)
      return "hcal_insert";
    return "hcal";
  }

  if (nametag.find("Track") != std::string::npos || nametag.find("track") != std::string::npos) {
    if (nametag.find("CKF") != std::string::npos)
      return "tracking_ckf";
    if (nametag.find("Seed") != std::string::npos)
      return "tracking_seeding";
    if (nametag.find("Vertex") != std::string::npos)
      return "tracking_vertex";
    return "tracking";
  }

  if (nametag.find("Cluster") != std::string::npos ||
      nametag.find("cluster") != std::string::npos) {
    return "clustering";
  }

  if (nametag.find("PID") != std::string::npos || nametag.find("Pid") != std::string::npos) {
    return "pid";
  }

  if (nametag.find("Jet") != std::string::npos || nametag.find("jet") != std::string::npos) {
    return "jets";
  }

  if (nametag.find("Truth") != std::string::npos || nametag.find("MC") != std::string::npos) {
    return "truth";
  }

  if (nametag.find("B0") != std::string::npos || nametag.find("ZDC") != std::string::npos) {
    return "far_detectors";
  }

  if (nametag.find("RICH") != std::string::npos) {
    return "rich";
  }

  if (nametag.find("TOF") != std::string::npos) {
    return "tof";
  }

  if (nametag.find("DIRC") != std::string::npos) {
    return "dirc";
  }

  if (nametag.find("GEM") != std::string::npos || nametag.find("MPGD") != std::string::npos) {
    return "gem_tracking";
  }

  if (nametag.find("Silicon") != std::string::npos || nametag.find("Pixel") != std::string::npos) {
    return "silicon_tracking";
  }

  if (nametag.find("Processor") != std::string::npos) {
    return "processors";
  }

  if (nametag.find("Source") != std::string::npos) {
    return "io";
  }

  // Default fallback - try to use first part of name
  size_t colon_pos = nametag.find(':');
  if (colon_pos != std::string::npos) {
    std::string base_name = nametag.substr(0, colon_pos);
    // Convert to lowercase for consistency
    std::transform(base_name.begin(), base_name.end(), base_name.begin(), ::tolower);
    return base_name;
  }

  // Final fallback
  return "misc";
}
