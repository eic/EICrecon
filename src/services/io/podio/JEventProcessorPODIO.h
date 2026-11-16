
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <podio/Writer.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <mutex>
#include <regex>
#include <set>
#include <string>
#include <vector>

class JEventProcessorPODIO : public JEventProcessor {

public:
  JEventProcessorPODIO();
  virtual ~JEventProcessorPODIO() = default;

  void Init() override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void Finish() override;

  void FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event);

  struct CollectionMatch {
    std::vector<std::string> matched;            // matching and not excluded, in sorted order
    std::vector<std::string> unmatched_patterns; // patterns that match no collection
  };
  // Match collection names against regex patterns in one pass
  static CollectionMatch MatchCollections(const std::set<std::string>& collections,
                                          const std::set<std::string>& patterns,
                                          const std::set<std::string>& excluded) {
    std::vector<std::regex> regexes(patterns.begin(), patterns.end());
    std::vector<bool> used(regexes.size(), false);

    CollectionMatch result;
    for (const auto& c : collections) {
      bool matched = false;
      for (std::size_t i = 0; i < regexes.size(); ++i) {
        // once c matched, only patterns that matched nothing yet need testing
        if (matched && used[i]) {
          continue;
        }
        if (std::regex_match(c, regexes[i])) {
          matched = true;
          used[i] = true;
        }
      }
      if (matched && !excluded.contains(c)) {
        result.matched.push_back(c);
      }
    }

    std::size_t i = 0;
    for (const auto& p : patterns) {
      if (!used[i++]) {
        result.unmatched_patterns.push_back(p);
      }
    }
    return result;
  }

protected:
  /// Propagate all non-"events" frames from any JEventSourcePODIO input source(s)
  /// to the output file.
  void PropagateNonEventCategories();

public:
  std::unique_ptr<podio::Writer> m_writer;
  std::mutex m_mutex;
  std::once_flag m_is_first_event;
  std::shared_ptr<spdlog::logger> m_log;

  std::string m_output_file          = "podio_output.root";
  std::string m_output_file_copy_dir = "";
  std::string m_output_backend       = "root";
  std::set<std::string> m_output_collections;         // config. parameter
  std::set<std::string> m_output_exclude_collections; // config. parameter
  std::vector<std::string> m_collections_to_write;    // derived from above config. parameters
  std::vector<std::string> m_collections_to_print;
};
