// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <podio/Reader.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <set>
#include <string>

class JEventSourcePODIO : public JEventSource {

public:
  JEventSourcePODIO(std::string resource_name, JApplication* app);

  virtual ~JEventSourcePODIO();

  void Open() override;

  void Close() override;

  Result Emit(JEvent& event) override;

  static std::string GetDescription();

  void PrintCollectionTypeTable(void);

protected:
  std::unique_ptr<podio::Reader> m_reader;

  std::size_t Nevents_in_file = 0;
  std::size_t Nevents_read    = 0;

  bool m_run_forever       = false;
  bool m_use_event_headers = true;

  std::set<std::string> m_input_collections;     // config. parameter

private:
  std::shared_ptr<spdlog::logger> m_log;
};

template <> double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);
