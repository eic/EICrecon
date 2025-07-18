// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Wouter Deconinck

#include <JANA/JEventProcessor.h>

#include <map>
#include <string>

class JEventProcessorJANATOP : public JEventProcessor {
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
  JEventProcessorJANATOP() : JEventProcessor() { SetTypeName("JEventProcessorJANATOP"); };

  void Init() override {};

  void BeginRun(const std::shared_ptr<const JEvent>& /* event */) override {};

  void Process(const std::shared_ptr<const JEvent>& event) override {
    // Get the call stack for ths event and add the results to our stats
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
  };

  void EndRun() override {};

  void Finish() override {
    // In order to get the total time we have to first get a list of
    // the event processors (i.e. top-level callers). We can tell
    // this just by looking for callers that never show up as callees
    std::set<std::string> callers;
    std::set<std::string> callees;
    for (auto iter = call_links.begin(); iter != call_links.end(); iter++) {
      const CallLink& link = iter->first;
      std::string caller   = MakeNametag(link.caller_name, link.caller_tag);
      std::string callee   = MakeNametag(link.callee_name, link.callee_tag);
      callers.insert(caller);
      callees.insert(callee);
    }

    // Loop over list a second time so we can get the total ticks for
    // the process in order to add the percentage to the label below
    double total_ms = 0.0;
    for (auto iter = call_links.begin(); iter != call_links.end(); iter++) {
      const CallLink& link   = iter->first;
      const CallStats& stats = iter->second;
      std::string caller     = MakeNametag(link.caller_name, link.caller_tag);
      std::string callee     = MakeNametag(link.callee_name, link.callee_tag);
      if (callees.find(caller) == callees.end()) {
        total_ms += stats.from_factory_ms + stats.from_source_ms + stats.from_cache_ms +
                    stats.data_not_available_ms;
      }
    }
    if (total_ms == 0.0)
      total_ms = 1.0;

    // Loop over call links
    std::cout << "Links:" << std::endl;
    std::vector<std::pair<CallLink, CallStats>> call_links_vector{
        std::make_move_iterator(std::begin(call_links)),
        std::make_move_iterator(std::end(call_links))};
    [[maybe_unused]] auto call_links_compare_time = [](const auto& a, const auto& b) {
      return ((a.second.from_factory_ms + a.second.from_source_ms + a.second.from_cache_ms) <
              (b.second.from_factory_ms + b.second.from_source_ms + b.second.from_cache_ms));
    };
    [[maybe_unused]] auto call_links_compare_N = [](const auto& a, const auto& b) {
      return ((a.second.Nfrom_factory + a.second.Nfrom_source + a.second.Nfrom_cache) <
              (b.second.Nfrom_factory + b.second.Nfrom_source + b.second.Nfrom_cache));
    };
    std::sort(call_links_vector.begin(), call_links_vector.end(), call_links_compare_time);
    for (auto iter = call_links_vector.end() - std::min(call_links_vector.size(), 10ul);
         iter != call_links_vector.end(); iter++) {
      const CallLink& link   = iter->first;
      const CallStats& stats = iter->second;

      unsigned int Ntotal = stats.Nfrom_cache + stats.Nfrom_source + stats.Nfrom_factory;

      std::string nametag1 = MakeNametag(link.caller_name, link.caller_tag);
      std::string nametag2 = MakeNametag(link.callee_name, link.callee_tag);

      double this_ms      = stats.from_factory_ms + stats.from_source_ms + stats.from_cache_ms;
      std::string timestr = MakeTimeString(stats.from_factory_ms);
      double percent      = 100.0 * this_ms / total_ms;
      char percentstr[32];
      snprintf(percentstr, 32, "%5.1f%%", percent);

      std::cout << Ntotal << " calls, " << timestr << " (" << percentstr << ") ";
      std::cout << nametag1 << " -> " << nametag2;
      std::cout << std::endl;
    }

    // Loop over factories
    std::cout << "Factories:" << std::endl;
    std::vector<std::pair<std::string, FactoryCallStats>> factory_stats_vector{
        std::make_move_iterator(std::begin(factory_stats)),
        std::make_move_iterator(std::end(factory_stats))};
    auto factory_stats_compare = [](const auto& a, const auto& b) {
      return ((a.second.time_waited_on - a.second.time_waiting) <
              (b.second.time_waited_on - b.second.time_waiting));
    };
    std::sort(factory_stats_vector.begin(), factory_stats_vector.end(), factory_stats_compare);
    for (auto iter = factory_stats_vector.end() - std::min(factory_stats_vector.size(), 10ul);
         iter != factory_stats_vector.end(); iter++) {
      FactoryCallStats& fcall_stats = iter->second;
      std::string nodename          = iter->first;

      // Get time spent in this factory proper
      double time_spent_in_factory = fcall_stats.time_waited_on - fcall_stats.time_waiting;
      std::string timestr          = MakeTimeString(time_spent_in_factory);
      double percent               = 100.0 * time_spent_in_factory / total_ms;
      char percentstr[32];
      snprintf(percentstr, 32, "%5.1f%%", percent);

      std::cout << timestr << " (" << percentstr << ") ";
      std::cout << nodename;
      std::cout << std::endl;
    }
  };

private:
  std::mutex mutex;

  std::map<CallLink, CallStats> call_links;
  std::map<std::string, FactoryCallStats> factory_stats;

  std::string MakeTimeString(double time_in_ms) {
    double order = log10(time_in_ms);
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

  std::string MakeNametag(const std::string& name, const std::string& tag) {
    std::string nametag = name;
    if (tag.size() > 0)
      nametag += ":" + tag;
    return nametag;
  }
};
