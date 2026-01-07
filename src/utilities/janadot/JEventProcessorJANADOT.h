// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, EICrecon contributors

#pragma once

#include <JANA/JEventProcessor.h>
#include <compare>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

class JEventProcessorJANADOT : public JEventProcessor {
private:
  enum node_type { kDefault, kProcessor, kFactory, kCache, kSource };

  class CallLink {
  public:
    std::string caller_name;
    std::string caller_tag;
    std::string callee_name;
    std::string callee_tag;

    bool operator<(const CallLink& link) const {
      if (this->caller_name != link.caller_name)
        return this->caller_name < link.caller_name;
      if (this->callee_name != link.callee_name)
        return this->callee_name < link.callee_name;
      if (this->caller_tag != link.caller_tag)
        return this->caller_tag < link.caller_tag;
      return this->callee_tag < link.callee_tag;
    }
  };

  class CallStats {
  public:
    CallStats(void) {
      from_cache_ms         = 0;
      from_source_ms        = 0;
      from_factory_ms       = 0;
      data_not_available_ms = 0;
      Nfrom_cache           = 0;
      Nfrom_source          = 0;
      Nfrom_factory         = 0;
      Ndata_not_available   = 0;
    }
    double from_cache_ms;
    double from_source_ms;
    double from_factory_ms;
    double data_not_available_ms;
    unsigned int Nfrom_cache;
    unsigned int Nfrom_source;
    unsigned int Nfrom_factory;
    unsigned int Ndata_not_available;
  };

  class FactoryCallStats {
  public:
    FactoryCallStats(void) {
      type           = kDefault;
      time_waited_on = 0.0;
      time_waiting   = 0.0;
      Nfrom_factory  = 0;
      Nfrom_source   = 0;
      Nfrom_cache    = 0;
    }
    node_type type;
    double time_waited_on; // time other factories spent waiting on this factory
    double time_waiting;   // time this factory spent waiting on other factories
    unsigned int Nfrom_factory;
    unsigned int Nfrom_source;
    unsigned int Nfrom_cache;
  };

public:
  JEventProcessorJANADOT() : JEventProcessor() { SetTypeName("JEventProcessorJANADOT"); };

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& /* event */) override {};
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override {};
  void Finish() override;

private:
  std::mutex mutex;

  std::map<CallLink, CallStats> call_links;
  std::map<std::string, FactoryCallStats> factory_stats;
  std::map<std::string, std::string> nametag_to_plugin; // Maps nametag to plugin name

  // Configuration parameters
  std::string output_filename;
  bool enable_splitting;

  // Group-related parameters
  std::map<std::string, std::vector<std::string>> user_groups; // Group name -> list of factories
  std::map<std::string, std::string> user_group_colors;        // Group name -> color
  std::map<std::string, std::string> nametag_to_group;         // Nametag -> group name

  // Helper methods
  std::string MakeTimeString(double time_in_ms);
  std::string MakeNametag(const std::string& name, const std::string& tag);
  node_type GetNodeType(const std::string& name, const std::string& tag);
  std::string GetNodeColor(node_type type);
  std::string GetNodeShape(node_type type);

  // DOT file generation methods
  void WriteDotFile();
  void WriteSingleDotFile(const std::string& filename);
  void WriteSplitDotFiles();
  void WritePluginGraphs(const std::map<std::string, std::set<std::string>>& plugin_groups);
  void WritePluginDotFile(const std::string& plugin_name, const std::set<std::string>& nodes);
  void WriteOverallDotFile(const std::map<std::string, std::set<std::string>>& plugin_groups);
  std::map<std::string, std::set<std::string>> SplitGraphByPlugin();
};
