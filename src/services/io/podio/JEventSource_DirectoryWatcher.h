// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include "DirectoryWatcher.h"
#include <podio/ROOTReader.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <string>


class JEventSource_DirectoryWatcher : public JEventSource {

public:
  JEventSource_DirectoryWatcher();
  ~JEventSource_DirectoryWatcher() override;

  void Init() override;
  void Open() override;
  void Close() override;
  Result Emit(JEvent& event) override;

  static std::string GetDescription();
  void OpenFile(std::string filename);

private:
  std::unique_ptr<podio::ROOTReader> m_current_reader;
  std::string m_current_filename;

  std::size_t m_events_in_file = 0;
  std::size_t m_events_emitted = 0;

  bool m_use_event_headers = true;

  DirectoryWatcher m_directory_watcher;

  std::shared_ptr<spdlog::logger> m_log;
};

template <> double JEventSourceGeneratorT<JEventSource_DirectoryWatcher>::CheckOpenable(std::string resource_name);


