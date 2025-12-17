// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <podio/ROOTReader.h>
#include <podio/Frame.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <string>

#if ((JANA_VERSION_MAJOR == 2) && (JANA_VERSION_MINOR >= 3)) || (JANA_VERSION_MAJOR > 2)
#define JANA_NEW_CALLBACK_STYLE 1
#else
#define JANA_NEW_CALLBACK_STYLE 0
#endif

class PodioRunFrame_service;

class JEventSourcePODIO : public JEventSource {

public:
  JEventSourcePODIO(std::string resource_name, JApplication* app);

  virtual ~JEventSourcePODIO();

  void Open() override;

  void Close() override;

#if JANA_NEW_CALLBACK_STYLE
  Result Emit(JEvent& event) override;
#else
  void GetEvent(std::shared_ptr<JEvent>) override;
#endif

  static std::string GetDescription();

  void PrintCollectionTypeTable(void);

protected:
  podio::ROOTReader m_reader;

  std::size_t Nevents_in_file = 0;
  std::size_t Nevents_read    = 0;

  bool m_run_forever       = false;
  bool m_use_event_headers = true;

  // Run metadata handling
  int32_t m_last_emitted_run_number = -1;
  std::shared_ptr<podio::Frame> m_cached_run_frame;

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<PodioRunFrame_service> m_run_frame_service;
  
};

template <> double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string);
