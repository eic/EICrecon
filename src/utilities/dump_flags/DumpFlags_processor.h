#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include "extensions/spdlog/SpdlogMixin.h"

class DumpFlags_processor : public JEventProcessor, public eicrecon::SpdlogMixin {
public:
  //----------------------------
  // Init
  //
  // This is called once before the first call to the Process method
  // below. You may, for example, want to open an output file here.
  // Only one thread will call this.
  void Init() override;

  //----------------------------
  // Process
  //
  // This is called for every event. Multiple threads may call this
  // simultaneously. If you write something to an output file here
  // then make sure to protect it with a mutex or similar mechanism.
  // Minimize what is done while locked since that directly affects
  // the multi-threaded performance.
  void Process(const std::shared_ptr<const JEvent>& event) override;

  //----------------------------
  // Finish
  //
  // This is called once after all events have been processed. You may,
  // for example, want to close an output file here.
  // Only one thread will call this.
  void Finish() override;

private:
  /// If not empty, a python eicrecon run file is created
  std::string m_python_file_name = "";

  /// If not null, such markdown file is created
  std::string m_markdown_file_name = "";

  /// If not null, such json file is created
  std::string m_json_file_name = "";

  /// If not null, such jana configuration file is created
  std::string m_janaconfig_file_name = "jana.conf";

  /// Print parameter summary to screen at end of job
  bool m_print_to_screen = true;

  /// Prefixes of flags that belongs to reconstruction parameters
  std::vector<std::string> m_reco_prefixes = {
      "B0TRK", "BEMC",  "DRICH", "BTRK", "BVTX",     "ECTRK", "EEMC", "FOFFMTRK",   "HCAL",
      "MPGD",  "RPOTS", "LOWQ2", "ZDC",  "Tracking", "Reco",  "Digi", "Calorimetry"};

  /// Checks if flags starts with one of m_reco_prefixes
  bool isReconstructionFlag(
      std::string
          flag_name) { // (!) copy value is important here! don't do const& NOLINT(performance-unnecessary-value-param)

    // convert flag_name to lower
    std::transform(flag_name.begin(), flag_name.end(), flag_name.begin(),
                   static_cast<int (*)(int)>(&std::tolower));

    for (auto subsystem : m_reco_prefixes) { // (!) copy value is important here! don't do auto&

      // Convert subsystem to lower
      std::transform(subsystem.begin(), subsystem.end(), subsystem.begin(),
                     static_cast<int (*)(int)>(&std::tolower));

      // if not sure, read this
      // https://stackoverflow.com/questions/1878001/how-do-i-check-if-a-c-stdstring-starts-with-a-certain-string-and-convert-a
      if (flag_name.rfind(subsystem, 0) == 0) { // pos=0 limits the search to the prefix
        // s starts with prefix
        return true;
      }
    }

    // flag prefix is not in list
    return false;
  }

  std::string findCategory(
      std::string
          flag_name) { // (!) copy value is important here! don't do const& NOLINT(performance-unnecessary-value-param)

    // convert flag_name to lower
    std::transform(flag_name.begin(), flag_name.end(), flag_name.begin(),
                   static_cast<int (*)(int)>(&std::tolower));

    for (auto subsystem : m_reco_prefixes) { // (!) copy value is important here! don't do auto&

      // Convert subsystem to lower
      std::string original_subsystem_name = subsystem;
      std::transform(subsystem.begin(), subsystem.end(), subsystem.begin(),
                     static_cast<int (*)(int)>(&std::tolower));

      // if not sure, read this
      // https://stackoverflow.com/questions/1878001/how-do-i-check-if-a-c-stdstring-starts-with-a-certain-string-and-convert-a
      if (flag_name.rfind(subsystem, 0) == 0) { // pos=0 limits the search to the prefix
        // s starts with prefix
        return original_subsystem_name;
      }
    }

    // flag prefix is not in list
    return "";
  }
};
