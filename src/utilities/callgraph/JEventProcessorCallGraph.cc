// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, EICrecon contributors

#include "JEventProcessorCallGraph.h"

#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <JANA/JFactory.h>
#include <JANA/JFactorySet.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JCallGraphRecorder.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <format>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ratio>
#include <sstream>
#include <utility>

void JEventProcessorCallGraph::Init() {
  // Get parameter manager
  auto params = GetApplication()->GetJParameterManager();

  // Set default parameter values and register them
  output_filename = "callgraph.dot";
  params->SetDefaultParameter("callgraph:output_file", output_filename, "Output DOT filename");

  enable_splitting = true;
  params->SetDefaultParameter("callgraph:enable_splitting", enable_splitting,
                              "Enable splitting graphs into multiple files by plugin");

  // Check for callgraph:group parameters (command line group definitions)
  // These override the default plugin-based group assignment
  std::map<std::string, std::string> parameter_keys;
  params->FilterParameters(parameter_keys, "callgraph:group:");
  for (const auto& [group_name, group_definition] : parameter_keys) {
    // Parse command line group definition similar to file format
    std::vector<std::string> factories;
    std::string color = "lightblue"; // default color

    std::stringstream ss(group_definition);
    std::string item;
    while (std::getline(ss, item, ',')) {
      // Trim surrounding whitespace, e.g. "Type:Tag1, Type:Tag2, color_blue"
      std::size_t first = item.find_first_not_of(" \t");
      if (first == std::string::npos) {
        continue; // all whitespace
      }
      std::size_t last = item.find_last_not_of(" \t");
      item             = item.substr(first, last - first + 1);

      if (item.find("color_") == 0) {
        color = item.substr(6); // remove "color_" prefix
      } else if (!item.empty()) {
        factories.push_back(item);
      }
    }

    user_group_colors[group_name] = color;

    // Build nametag to group mapping (overrides plugin-based assignment).
    // Each factory spec is "Tag" or "Type:Tag" (the type itself may contain "::",
    // so split on the last ':'). This map is later looked up by the nametags that
    // MakeNametag() makes from the call graph: "tag (type)" for factories with an
    // object name (JFactoryT-based ones), but just "tag" for the others, which
    // include every JOmniFactory, so register both forms.
    for (const auto& factory : factories) {
      auto colon_pos        = factory.rfind(':');
      std::string type_part = (colon_pos == std::string::npos) ? "" : factory.substr(0, colon_pos);
      std::string tag_part =
          (colon_pos == std::string::npos) ? factory : factory.substr(colon_pos + 1);
      std::string nametag        = MakeNametag(type_part, tag_part);
      nametag_to_group[nametag]  = group_name;
      nametag_to_group[tag_part] = group_name;
      user_group_entries.push_back({group_name, factory, {nametag, tag_part}});
    }
  }

  // Add default group assignments for special factories
  // podio::Frame is a PODIO framework factory that has no plugin name
  nametag_to_group["podio::Frame"] = "processors";
}

void JEventProcessorCallGraph::Process(const std::shared_ptr<const JEvent>& event) {
  // Get the call stack for this event and add the results to our stats
  auto stack = event->GetJCallGraphRecorder()->GetCallGraph();

  // Lock mutex in case we are running with multiple threads
  std::lock_guard<std::mutex> lck(mutex);
  ++event_count; // reported in the legend

  // Build mapping of nametags to plugin names (only do this once per instance)
  if (!factory_mapping_built) {
    auto factories = event->GetFactorySet()->GetAllFactories();

    for (auto* factory : factories) {
      std::string nametag     = MakeNametag(factory->GetObjectName(), factory->GetTag());
      std::string plugin_name = factory->GetPluginName();

      // If plugin name is empty, use "core" as default
      if (plugin_name.empty()) {
        plugin_name = "core";
      }

      nametag_to_plugin[nametag] = plugin_name;
    }

    // Group tags by finding sets of tags that share a common prefix
    // where the common prefix exactly matches one of the tags in the set
    std::map<std::string, std::vector<std::string>> prefix_to_tags;
    std::vector<std::string> all_helper_tags;
    std::set<std::string> processed_tags;

    for (auto* factory : factories) {
      std::string factory_name = factory->GetFactoryName();
      std::string factory_tag  = factory->GetTag();

      // Helper factories can legitimately have an empty tag. An empty string
      // is a "prefix" of every other string (find("") == 0), so including it
      // here would collapse every helper tag into a single bogus group.
      if (factory_name.find("::Helper<") != std::string::npos && !factory_tag.empty()) {
        all_helper_tags.push_back(factory_tag);
      }
    }

    // Sort tags by length (shortest first) to ensure prefixes come before their extensions,
    // with a lexical tie-breaker so equal-length tags sort deterministically across runs.
    std::sort(all_helper_tags.begin(), all_helper_tags.end(),
              [](const std::string& a, const std::string& b) {
                if (a.length() != b.length())
                  return a.length() < b.length();
                return a < b;
              });

    // Group tags by finding their longest common prefix
    // For each pair/group of tags that share a common prefix, group them together
    for (std::size_t i = 0; i < all_helper_tags.size(); ++i) {
      if (processed_tags.count(all_helper_tags[i]))
        continue;

      std::vector<std::string> group;
      group.push_back(all_helper_tags[i]);

      // Find all tags that differ from all_helper_tags[i] only in a suffix
      for (std::size_t j = i + 1; j < all_helper_tags.size(); ++j) {
        if (processed_tags.count(all_helper_tags[j]))
          continue;

        // Check if one tag is a prefix of the other, OR they share all but the last word
        // Strategy: one should start with the other, OR they should be identical except for a suffix
        bool should_group = false;

        // Case 1: tag j starts with tag i (tag i is a prefix)
        if (all_helper_tags[j].find(all_helper_tags[i]) == 0) {
          should_group = true;
        }
        // Case 2: they share a common prefix and differ only at the end
        else {
          // Find common prefix
          std::size_t k = 0;
          while (k < all_helper_tags[i].length() && k < all_helper_tags[j].length() &&
                 all_helper_tags[i][k] == all_helper_tags[j][k]) {
            ++k;
          }

          // They should share at least 90% of the shorter tag's length
          std::size_t min_len = std::min(all_helper_tags[i].length(), all_helper_tags[j].length());
          if (k >= min_len * 0.9 && k >= 15) {
            should_group = true;
          }
        }

        if (should_group) {
          group.push_back(all_helper_tags[j]);
          processed_tags.insert(all_helper_tags[j]);
        }
      }

      if (group.size() > 1) {
        // Use the first tag (shortest) as the factory_id
        prefix_to_tags[all_helper_tags[i]] = group;
        processed_tags.insert(all_helper_tags[i]);
      }
    }

    // Build tag_to_factory_id mapping
    std::map<std::string, std::string> tag_to_factory_id;
    for (const auto& [prefix, tags] : prefix_to_tags) {
      for (const auto& tag : tags) {
        tag_to_factory_id[tag] = prefix;
      }
    }

    // Map each nametag to its factory_id
    for (auto* factory : factories) {
      std::string nametag     = MakeNametag(factory->GetObjectName(), factory->GetTag());
      std::string factory_tag = factory->GetTag();
      // Default to tag, but an empty tag is valid (MakeNametag falls back to
      // the object name); use the nametag then so distinct such factories
      // don't collapse into a single "" factory_id.
      std::string factory_id = factory_tag.empty() ? nametag : factory_tag;

      // Check if this tag has a factory_id mapping (from prefix grouping)
      auto it = tag_to_factory_id.find(factory_tag);
      if (it != tag_to_factory_id.end()) {
        factory_id = it->second;
      }

      // Track all output tags for this factory_id. An empty tag is valid (see
      // above), but storing "" here would later render as a blank line in the
      // "Outputs:" list, so fall back to factory_id for the stored value too.
      std::string output_tag = factory_tag.empty() ? factory_id : factory_tag;
      if (factory_outputs.find(factory_id) == factory_outputs.end()) {
        factory_outputs[factory_id] = {output_tag};
      } else {
        auto& tags = factory_outputs[factory_id];
        if (std::find(tags.begin(), tags.end(), output_tag) == tags.end()) {
          tags.push_back(output_tag);
        }
      }

      // Map this nametag to the factory_id for grouping
      nametag_to_factory_id[nametag] = factory_id;
    }

    // GetAllFactories() iteration order isn't guaranteed stable, so sort each
    // factory's output tags to keep the generated DOT/SVG output deterministic.
    for (auto& [factory_id, tags] : factory_outputs) {
      std::sort(tags.begin(), tags.end());
    }

    factory_mapping_built = true;
  }

  // Loop over the call stack elements and add in the values
  for (std::size_t i = 0; i < stack.size(); ++i) {

    // Keep track of total time each factory spent waiting and being waited on
    std::string nametag1 = MakeNametag(stack[i].caller_name, stack[i].caller_tag);
    std::string nametag2 = MakeNametag(stack[i].callee_name, stack[i].callee_tag);

    FactoryCallStats& fcallstats1 = factory_stats[nametag1];
    FactoryCallStats& fcallstats2 = factory_stats[nametag2];

    // Determine node types
    fcallstats1.type = GetNodeType(stack[i].caller_name, stack[i].caller_tag);
    fcallstats2.type = GetNodeType(stack[i].callee_name, stack[i].callee_tag);

    // Use a floating-point duration so calls shorter than 1ms still contribute
    // their actual time instead of being truncated to 0.
    auto delta_t_ms =
        std::chrono::duration<double, std::milli>(stack[i].end_time - stack[i].start_time).count();
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

void JEventProcessorCallGraph::Finish() { WriteDotFile(); }

void JEventProcessorCallGraph::WriteGraphLabel(std::ostream& ofs, const std::string& title,
                                               graph_type type) {
  // The graph label holds the title and a legend defining the quantities in the
  // node and edge labels. Unlike a legend node, it takes no part in the layout.
  // Each legend row is a term and the lines of its definition.
  std::vector<std::pair<std::string, std::vector<std::string>>> rows;
  if (type == kOverviewGraph) {
    rows.push_back({"node",
                    {
                        user_group_colors.empty()
                            ? "one box per plugin"
                            : "one box per plugin or user-defined group (colored border)",
                        "label: name, number of factories, plugin time (% of T)",
                        "click it to open its graph",
                    }});
    rows.push_back({"A -&gt; B",
                    {
                        "factories in A requested data from factories in B",
                        "label: calls, edge time (% of T), summed over these requests",
                    }});
    rows.push_back({"plugin time",
                    {
                        "sum of the self times of its factories",
                        "= sum of incoming edge times - sum of outgoing edge times",
                        "(requests between factories in the same plugin cancel out)",
                    }});
  } else {
    rows.push_back(
        {"node",
         {
             "label: name, Outputs (if several), Inputs (in this graph), self time (% of T)",
             "box: factory, ellipse: event processor or other,",
             "diamond: cache, trapezium: event source",
         }});
    rows.push_back({"A -&gt; B",
                    {
                        "A requested data from B",
                        "label: calls, edge time (% of T)",
                    }});
  }
  rows.push_back({"self time",
                  {
                      "time spent in a node's own code, not waiting for its inputs",
                      "= sum of incoming edge times - sum of outgoing edge times.",
                      "Not measured for event processors (shown as 0);",
                      "the edges leaving them add up to T.",
                  }});
  rows.push_back({"edge time",
                  {
                      "time the caller waited for the requested data, summed over all calls.",
                      "A factory runs once per event, for the first request; that request's time",
                      "includes the factory's self time and all of its outgoing edges, while",
                      "later requests are served from cache and add almost nothing.",
                  }});
  rows.push_back({"calls", {"number of requests, including those served from cache"}});
  rows.push_back({"T = 100%",
                  {
                      std::format("{}, the self time of all nodes in all plugins,",
                                  MakeTimeString(GetTotalSelfTime())),
                      std::format("summed over {} events (on all threads)", event_count),
                  }});
  if (type == kPluginGraph) {
    rows.push_back({"not drawn",
                    {
                        "requests to or from other plugins (see the overview graph);",
                        "they still count in the sums above",
                    }});
  }
  // Node fill colors, drawn with the same mapping the nodes use
  std::string swatches = "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\"><TR>";
  for (int percent : {0, 25, 50, 75, 100}) {
    swatches +=
        std::format("<TD BGCOLOR=\"{}\">{}%</TD>", GetNodeColorFromPercent(percent), percent);
  }
  swatches += "</TR></TABLE>";
  rows.push_back({"node fill", {swatches}});
  rows.push_back({"edge width", {"from 1 (0%) to 8 (100%)"}});

  ofs << "  fontname=\"Arial\";" << std::endl;
  ofs << "  fontsize=10;" << std::endl;
  ofs << "  labelloc=\"t\";" << std::endl;
  ofs << "  labeljust=\"l\";" << std::endl;
  ofs << "  label=<<TABLE BORDER=\"0\" CELLSPACING=\"4\">" << std::endl;
  ofs << "    <TR><TD ALIGN=\"LEFT\"><FONT POINT-SIZE=\"16\">" << EscapeForHtml(title)
      << "</FONT></TD></TR>" << std::endl;
  ofs << "    <TR><TD ALIGN=\"LEFT\"><TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\" "
         "CELLPADDING=\"3\" COLOR=\"gray50\">"
      << std::endl;
  ofs << "      <TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\"><B>Legend</B></TD></TR>" << std::endl;
  for (const auto& [term, lines] : rows) {
    ofs << "      <TR><TD ALIGN=\"LEFT\" VALIGN=\"TOP\"><B>" << term
        << "</B></TD><TD ALIGN=\"LEFT\" BALIGN=\"LEFT\">";
    for (std::size_t i = 0; i < lines.size(); ++i) {
      ofs << (i > 0 ? "<BR/>" : "") << lines[i];
    }
    ofs << "</TD></TR>" << std::endl;
  }
  ofs << "    </TABLE></TD></TR>" << std::endl;
  ofs << "  </TABLE>>;" << std::endl;
}

void JEventProcessorCallGraph::WriteDotFile() {
  if (enable_splitting) {
    std::cout << "Splitting call graph into multiple files..." << std::endl;
    WriteSplitDotFiles();
  } else {
    WriteSingleDotFile(output_filename);
    std::cout << std::endl;
    std::cout << "Factory calling information written to \"" << output_filename
              << "\". To create a graphic" << std::endl;
    std::cout << "from this, use the dot program. For example, to make a PDF file do the following:"
              << std::endl;
    std::cout << std::endl;
    std::string pdf_filename = GetBaseFilename() + ".pdf";
    std::cout << "   dot -Tpdf " << output_filename << " -o " << pdf_filename << std::endl;
    std::cout << std::endl;
    std::cout << "This should give you a file named \"" << pdf_filename << "\"." << std::endl;
    std::cout << std::endl;
  }
}

void JEventProcessorCallGraph::WriteSingleDotFile(const std::string& filename) {
  std::cout << "Opening output file \"" << filename << "\"" << std::endl;

  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing!" << std::endl;
    return;
  }

  // Percentages are relative to the total self time of all nodes (see GetTotalSelfTime)
  double total_ms = GetTotalSelfTime();
  if (total_ms == 0.0)
    total_ms = 1.0;

  // Write DOT file header
  ofs << "digraph G {" << std::endl;
  ofs << "  rankdir=TB;" << std::endl;
  ofs << "  node [fontname=\"Arial\", fontsize=10];" << std::endl;
  ofs << "  edge [fontname=\"Arial\", fontsize=8];" << std::endl;
  WriteGraphLabel(ofs, "EICrecon Call Graph", kSingleGraph);
  ofs << std::endl;

  // Aggregate factory stats by factory ID (groups multi-output factories together)
  std::map<std::string, double> factory_times;
  std::map<std::string, std::vector<std::string>> factory_output_tags;
  std::map<std::string, node_type> factory_types;

  for (auto& [nametag, fstats] : factory_stats) {
    std::string factory_id = GetFactoryNodeName(nametag);
    double time_in_factory = GetSelfTime(fstats);

    factory_times[factory_id] += time_in_factory;
    factory_types[factory_id] = fstats.type;
  }

  // Populate output tags for each factory ID using the factory_outputs map
  for (auto& [factory_id, output_tags] : factory_outputs) {
    factory_output_tags[factory_id] = output_tags;
  }

  // Build input tags for each factory ID by examining call links
  std::map<std::string, std::set<std::string>> factory_input_tags;
  for (auto& [link, stats] : call_links) {
    std::string caller_nametag = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee_nametag = MakeNametag(link.callee_name, link.callee_tag);
    std::string caller_id      = GetFactoryNodeName(caller_nametag);
    std::string callee_id      = GetFactoryNodeName(callee_nametag);

    // The caller depends on the callee, so callee is an input to caller.
    // Multiple nametags can collapse into the same factory_id, so skip
    // self-dependencies - a node listing itself as its own input is just noise.
    if (caller_id != callee_id) {
      factory_input_tags[caller_id].insert(callee_id);
    }
  }

  // Write nodes (one per factory, listing all outputs and inputs)
  for (auto& [factory_id, time_in_factory] : factory_times) {
    double percent = 100.0 * time_in_factory / total_ms;

    std::string color = GetNodeColorFromPercent(percent);
    std::string shape = GetNodeShape(factory_types[factory_id]);

    ofs << "  \"" << EscapeForDot(factory_id) << "\" [";
    ofs << "fillcolor=\"" << color << "\", ";
    ofs << "style=filled, ";
    ofs << "shape=" << shape << ", ";
    ofs << "labeljust=l, ";
    ofs << "label=\"" << EscapeForDot(factory_id);

    // List output collections
    auto& output_tags = factory_output_tags[factory_id];
    if (!output_tags.empty() && (output_tags.size() > 1 || output_tags[0] != factory_id)) {
      ofs << "\\nOutputs:";
      for (const auto& tag : output_tags) {
        ofs << "\\n  " << EscapeForDot(tag);
      }
    }

    // List input collections
    auto it = factory_input_tags.find(factory_id);
    if (it != factory_input_tags.end() && !it->second.empty()) {
      ofs << "\\nInputs:";
      for (const auto& tag : it->second) {
        ofs << "\\n  " << EscapeForDot(tag);
      }
    }

    ofs << "\\n"
        << MakeTimeString(time_in_factory) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << std::endl;

  // Write edges (grouped by factory names)
  std::map<std::pair<std::string, std::string>, std::pair<unsigned int, double>> aggregated_links;

  for (auto& [link, stats] : call_links) {
    std::string caller_nametag = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee_nametag = MakeNametag(link.callee_name, link.callee_tag);
    std::string caller         = GetFactoryNodeName(caller_nametag);
    std::string callee         = GetFactoryNodeName(callee_nametag);

    unsigned int total_calls =
        stats.Nfrom_cache + stats.Nfrom_source + stats.Nfrom_factory + stats.Ndata_not_available;
    double total_time = stats.from_cache_ms + stats.from_source_ms + stats.from_factory_ms +
                        stats.data_not_available_ms;

    auto key = std::make_pair(caller, callee);
    aggregated_links[key].first += total_calls;
    aggregated_links[key].second += total_time;
  }

  for (auto& [link_pair, call_data] : aggregated_links) {
    unsigned int total_calls = call_data.first;
    double total_time        = call_data.second;
    double percent           = ClampPercent(100.0 * total_time / total_ms);

    ofs << "  \"" << EscapeForDot(link_pair.first) << "\" -> \"" << EscapeForDot(link_pair.second)
        << "\" [";
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

void JEventProcessorCallGraph::WriteSplitDotFiles() {
  // Always use plugin-based splitting with optional user group overrides
  std::map<std::string, std::set<std::string>> groups = SplitGraphByPlugin();

  // A group entry that matches nothing, e.g. a misspelled tag or a factory output other
  // than the first one (which the factory is named after), would otherwise be ignored silently
  for (const auto& [group_name, entry, nametags] : user_group_entries) {
    if (std::ranges::none_of(nametags, [this](const std::string& nametag) {
          return factory_stats.contains(nametag);
        })) {
      std::cerr << "Warning: callgraph:group:" << group_name << " entry \"" << entry
                << "\" matches nothing in the call graph and is ignored; use the name on a "
                   "factory's box in the graphs (for a factory with several outputs, the first one)"
                << std::endl;
    }
  }

  WritePluginGraphs(groups);
  WriteOverallDotFile(groups);
}

std::string JEventProcessorCallGraph::MakeTimeString(double time_in_ms) {
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

std::string JEventProcessorCallGraph::MakeNametag(const std::string& name, const std::string& tag) {
  std::string nametag = tag;
  if (tag.size() > 0 && name.size() > 0) {
    nametag += " (" + name + ")";
  } else if (name.size() > 0) {
    nametag = name;
  }
  return nametag;
}

std::string JEventProcessorCallGraph::GetBaseFilename() {
  // Strip the extension from output_filename so per-plugin files and their
  // cross-referenced URLs are named consistently, even when output_filename
  // isn't the default "callgraph.dot".
  std::string base_filename = output_filename;
  std::size_t dot_pos       = base_filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    base_filename = base_filename.substr(0, dot_pos);
  }
  return base_filename;
}

std::string JEventProcessorCallGraph::SanitizeForFilename(const std::string& name) {
  // Plugin/group names can come from user-supplied -Pcallgraph:group:<name>=...
  // parameters. Replace path separators so they can't escape the working
  // directory via the generated filename, and so the URL we embed in the
  // overview graph (built from the same sanitized name) always matches it.
  std::string sanitized = name;
  std::replace(sanitized.begin(), sanitized.end(), '/', '_');
  std::replace(sanitized.begin(), sanitized.end(), '\\', '_');
  return sanitized;
}

std::string JEventProcessorCallGraph::EscapeForDot(const std::string& s) {
  // color_<...> comes from user-supplied -Pcallgraph:group:*=... parameters and
  // is written into a quoted DOT attribute. Escape '"' and '\' so it can't
  // terminate the string early and inject additional attributes.
  std::string escaped;
  escaped.reserve(s.size());
  for (char c : s) {
    if (c == '"' || c == '\\') {
      escaped += '\\';
    }
    escaped += c;
  }
  return escaped;
}

std::string JEventProcessorCallGraph::EscapeForHtml(const std::string& s) {
  // The graph title includes plugin/group names, which can come from user-supplied
  // -Pcallgraph:group:<name>=... parameters, and goes into an HTML-like label (see
  // WriteGraphLabel). Escape the characters that would be parsed as markup there.
  std::string escaped;
  escaped.reserve(s.size());
  for (char c : s) {
    if (c == '&') {
      escaped += "&amp;";
    } else if (c == '<') {
      escaped += "&lt;";
    } else if (c == '>') {
      escaped += "&gt;";
    } else if (c == '"') {
      escaped += "&quot;";
    } else {
      escaped += c;
    }
  }
  return escaped;
}

double JEventProcessorCallGraph::ClampPercent(double percent) {
  return std::clamp(percent, 0.0, 100.0);
}

double JEventProcessorCallGraph::GetSelfTime(const FactoryCallStats& fstats) {
  // Time spent in the node's own code: the time its callers waited on it (the sum
  // of its incoming edges) minus the time it waited on its inputs (the sum of its
  // outgoing edges). A pure top-level caller (e.g. an event processor requesting
  // its outputs at end-of-event) is never waited on, so its own time isn't
  // measured and this is 0 for it. Its waiting time isn't a substitute: that's
  // the self time of the factories it requested, which would be counted twice.
  return std::max(0.0, fstats.time_waited_on - fstats.time_waiting);
}

double JEventProcessorCallGraph::GetTotalSelfTime() {
  // The 100% reference for the percentages in every graph: the self time of all
  // nodes, i.e. the time spent in all factories. Using the same total in each
  // plugin graph as in the overview keeps their percentages comparable, and keeps
  // edge times, which include callees in other plugins, within 100%.
  double total_ms = 0.0;
  for (auto& [nametag, fstats] : factory_stats) {
    total_ms += GetSelfTime(fstats);
  }
  return total_ms;
}

std::string JEventProcessorCallGraph::GetFactoryNodeName(const std::string& nametag) {
  // Return the factory ID for this nametag, which groups multi-output factories
  auto it = nametag_to_factory_id.find(nametag);
  if (it != nametag_to_factory_id.end()) {
    return it->second;
  }
  // Fallback to nametag if not found
  return nametag;
}

JEventProcessorCallGraph::node_type JEventProcessorCallGraph::GetNodeType(const std::string& name,
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

std::string JEventProcessorCallGraph::GetNodeColorFromPercent(double percent) {
  // Clamp percent to [0, 100]
  if (percent < 0.0)
    percent = 0.0;
  if (percent > 100.0)
    percent = 100.0;

  // Convert to [0, 1] range
  double t = percent / 100.0;

  // White (low/optimal) to Red (high) gradient
  // White: RGB(255, 255, 255) -> #FFFFFF at 0%
  // Red:   RGB(255, 0, 0) -> #FF0000 at 100%

  int r = 255;
  int g = static_cast<int>((1.0 - t) * 255);
  int b = static_cast<int>((1.0 - t) * 255);

  return std::format("#{:02X}{:02X}{:02X}", r, g, b);
}

std::string JEventProcessorCallGraph::GetNodeShape(node_type type) {
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

void JEventProcessorCallGraph::WritePluginGraphs(
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
  std::cout << "Plugin files use period-separated naming (e.g., callgraph.tracking.dot)."
            << std::endl;
  std::cout << "Overall summary in \"" << output_filename << "\" shows inter-plugin connections."
            << std::endl;
  std::cout << std::endl;
}

void JEventProcessorCallGraph::WritePluginDotFile(const std::string& plugin_name,
                                                  const std::set<std::string>& nodes) {
  // Create filename using period-separated plugin name
  std::string filename = GetBaseFilename() + "." + SanitizeForFilename(plugin_name) + ".dot";

  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for writing!" << std::endl;
    return;
  }

  // Percentages are relative to the total self time of all plugins, not just this
  // one (see GetTotalSelfTime)
  double total_ms = GetTotalSelfTime();
  if (total_ms == 0.0)
    total_ms = 1.0;

  // Write DOT file header
  ofs << "digraph G {" << std::endl;
  ofs << "  rankdir=TB;" << std::endl;
  ofs << "  node [fontname=\"Arial\", fontsize=10];" << std::endl;
  ofs << "  edge [fontname=\"Arial\", fontsize=8];" << std::endl;
  WriteGraphLabel(ofs, "EICrecon Call Graph - " + plugin_name + " Plugin", kPluginGraph);
  ofs << std::endl;

  // Aggregate factory stats by factory ID for nodes in this plugin
  std::map<std::string, double> factory_times;
  std::map<std::string, std::vector<std::string>> factory_output_tags;
  std::map<std::string, node_type> factory_types;
  std::set<std::string> factory_nodes; // Set of factory IDs in this plugin

  for (const std::string& nametag : nodes) {
    auto fstats_it = factory_stats.find(nametag);
    if (fstats_it == factory_stats.end())
      continue;

    const FactoryCallStats& fstats = fstats_it->second;
    std::string factory_id         = GetFactoryNodeName(nametag);
    double time_in_factory         = GetSelfTime(fstats);

    factory_times[factory_id] += time_in_factory;
    factory_types[factory_id] = fstats.type;
    factory_nodes.insert(factory_id);
  }

  // Populate output tags for each factory ID in this plugin
  for (const auto& factory_id : factory_nodes) {
    auto it = factory_outputs.find(factory_id);
    if (it != factory_outputs.end()) {
      factory_output_tags[factory_id] = it->second;
    }
  }

  // Build input tags for each factory ID in this plugin
  std::map<std::string, std::set<std::string>> factory_input_tags;
  for (auto& [link, stats] : call_links) {
    std::string caller_nametag = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee_nametag = MakeNametag(link.callee_name, link.callee_tag);

    // Only consider links where both the caller and callee are in this
    // plugin: this subgraph doesn't draw nodes/edges for external factories,
    // so an "Inputs:" list referencing one would point at nothing in the file.
    if (nodes.find(caller_nametag) != nodes.end() && nodes.find(callee_nametag) != nodes.end()) {
      std::string caller_id = GetFactoryNodeName(caller_nametag);
      std::string callee_id = GetFactoryNodeName(callee_nametag);
      // Multiple nametags can collapse into the same factory_id; skip
      // self-dependencies rather than listing a node as its own input.
      if (caller_id != callee_id) {
        factory_input_tags[caller_id].insert(callee_id);
      }
    }
  }

  // Write nodes (one per factory in this plugin, listing all outputs and inputs)
  for (auto& [factory_id, time_in_factory] : factory_times) {
    double percent = 100.0 * time_in_factory / total_ms;

    std::string color = GetNodeColorFromPercent(percent);
    std::string shape = GetNodeShape(factory_types[factory_id]);

    ofs << "  \"" << EscapeForDot(factory_id) << "\" [";
    ofs << "fillcolor=\"" << color << "\", ";
    ofs << "style=filled, ";
    ofs << "shape=" << shape << ", ";
    ofs << "labeljust=l, ";
    ofs << "label=\"" << EscapeForDot(factory_id);

    // List output collections
    auto& output_tags = factory_output_tags[factory_id];
    if (!output_tags.empty() && (output_tags.size() > 1 || output_tags[0] != factory_id)) {
      ofs << "\\nOutputs:";
      for (const auto& tag : output_tags) {
        ofs << "\\n  " << EscapeForDot(tag);
      }
    }

    // List input collections
    auto input_it = factory_input_tags.find(factory_id);
    if (input_it != factory_input_tags.end() && !input_it->second.empty()) {
      ofs << "\\nInputs:";
      for (const auto& tag : input_it->second) {
        ofs << "\\n  " << EscapeForDot(tag);
      }
    }

    ofs << "\\n"
        << MakeTimeString(time_in_factory) << " (" << std::fixed << std::setprecision(1) << percent
        << "%)\"";
    ofs << "];" << std::endl;
  }

  ofs << std::endl;

  // Write edges (aggregated by factory names, only within this plugin)
  std::map<std::pair<std::string, std::string>, std::pair<unsigned int, double>> aggregated_links;

  for (auto& [link, stats] : call_links) {
    std::string caller_nametag = MakeNametag(link.caller_name, link.caller_tag);
    std::string callee_nametag = MakeNametag(link.callee_name, link.callee_tag);

    // Only include edges where both nodes are in this plugin
    if (nodes.find(caller_nametag) == nodes.end() || nodes.find(callee_nametag) == nodes.end()) {
      continue;
    }

    std::string caller = GetFactoryNodeName(caller_nametag);
    std::string callee = GetFactoryNodeName(callee_nametag);

    unsigned int total_calls =
        stats.Nfrom_cache + stats.Nfrom_source + stats.Nfrom_factory + stats.Ndata_not_available;
    double total_time = stats.from_cache_ms + stats.from_source_ms + stats.from_factory_ms +
                        stats.data_not_available_ms;

    auto key = std::make_pair(caller, callee);
    aggregated_links[key].first += total_calls;
    aggregated_links[key].second += total_time;
  }

  for (auto& [link_pair, call_data] : aggregated_links) {
    unsigned int total_calls = call_data.first;
    double total_time        = call_data.second;
    double percent           = ClampPercent(100.0 * total_time / total_ms);

    ofs << "  \"" << EscapeForDot(link_pair.first) << "\" -> \"" << EscapeForDot(link_pair.second)
        << "\" [";
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

void JEventProcessorCallGraph::WriteOverallDotFile(
    const std::map<std::string, std::set<std::string>>& plugin_groups) {
  std::ofstream ofs(output_filename);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << output_filename << " for writing!" << std::endl;
    return;
  }

  std::string base_filename = GetBaseFilename();

  // Percentages are relative to the total self time of all nodes (see GetTotalSelfTime)
  double total_ms = GetTotalSelfTime();
  if (total_ms == 0.0)
    total_ms = 1.0;

  // Write DOT file header
  ofs << "digraph G {" << std::endl;
  ofs << "  rankdir=TB;" << std::endl;
  ofs << "  node [fontname=\"Arial\", fontsize=12];" << std::endl;
  ofs << "  edge [fontname=\"Arial\", fontsize=10];" << std::endl;
  WriteGraphLabel(ofs, "EICrecon Overall Call Graph - Inter-Plugin Connections", kOverviewGraph);
  ofs << std::endl;

  // Create plugin nodes (aggregate statistics per plugin)
  std::map<std::string, double> plugin_times;
  std::map<std::string, int> plugin_node_counts;

  for (auto& [plugin_name, nodes] : plugin_groups) {
    double plugin_time = 0.0;
    // Count unique factory node IDs, not nametags: multi-output factories
    // collapse multiple nametags into one node via GetFactoryNodeName(), so
    // counting nametags would inflate the displayed "N factories".
    std::set<std::string> unique_nodes;

    for (const std::string& nametag : nodes) {
      auto fstats_it = factory_stats.find(nametag);
      if (fstats_it != factory_stats.end()) {
        const FactoryCallStats& fstats = fstats_it->second;
        plugin_time += GetSelfTime(fstats);
        unique_nodes.insert(GetFactoryNodeName(nametag));
      }
    }
    int node_count = static_cast<int>(unique_nodes.size());

    plugin_times[plugin_name]       = plugin_time;
    plugin_node_counts[plugin_name] = node_count;

    double percent = 100.0 * plugin_time / total_ms;

    std::string color = GetNodeColorFromPercent(percent);

    ofs << "  \"" << EscapeForDot(plugin_name) << "\" [";
    ofs << "fillcolor=\"" << color << "\", ";
    ofs << "style=filled, ";
    // User-defined groups (via -Pcallgraph:group:) get their requested color as the
    // node border, so they're visually distinguishable from auto-detected plugins
    // while still using the time-percentage gradient for the fill.
    auto group_color_it = user_group_colors.find(plugin_name);
    if (group_color_it != user_group_colors.end()) {
      ofs << "color=\"" << EscapeForDot(group_color_it->second) << "\", penwidth=2, ";
    }
    ofs << "shape=box, ";
    ofs << "URL=\"" << EscapeForDot(base_filename) << "."
        << EscapeForDot(SanitizeForFilename(plugin_name)) << ".svg\", ";
    ofs << "label=\"" << EscapeForDot(plugin_name) << "\\n";
    ofs << node_count << " factories\\n";
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

    // Resolve group names the same way SplitGraphByPlugin() does (user group
    // overrides first, then plugin, then the "processors" fallback), so a
    // factory reassigned via -Pcallgraph:group:* contributes edges to its new
    // group instead of its original plugin.
    std::string caller_plugin = ResolveGroupName(caller);
    std::string callee_plugin = ResolveGroupName(callee);

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
    double percent    = ClampPercent(100.0 * total_time / total_ms);

    ofs << "  \"" << EscapeForDot(plugins.first) << "\" -> \"" << EscapeForDot(plugins.second)
        << "\" [";
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

std::string JEventProcessorCallGraph::ResolveGroupName(const std::string& nametag) {
  // Check if this factory has a user-defined group override
  auto override_it = nametag_to_group.find(nametag);
  if (override_it != nametag_to_group.end()) {
    return override_it->second;
  }
  // Use plugin name from factory
  auto it = nametag_to_plugin.find(nametag);
  if (it != nametag_to_plugin.end()) {
    return it->second;
  }
  // Call-graph nodes aren't limited to registered factories - event
  // processors (e.g. JEventProcessorPODIO requesting its outputs at
  // end-of-event) show up here too but have no GetAllFactories() entry.
  // Group them with other non-factory framework nodes rather than an
  // "unknown" bucket that would mix them with genuinely unmapped ones.
  return "processors";
}

std::map<std::string, std::set<std::string>> JEventProcessorCallGraph::SplitGraphByPlugin() {
  std::map<std::string, std::set<std::string>> plugin_groups;

  // Group nodes by their actual plugin (from factory information)
  // with optional user group overrides
  for (auto& [nametag, fstats] : factory_stats) {
    std::string group_name = ResolveGroupName(nametag);
    plugin_groups[group_name].insert(nametag);
  }

  return plugin_groups;
}
