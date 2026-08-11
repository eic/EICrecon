// Copyright 2024, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// Apache Arrow IPC Stream Reader for EDM4hep
// Compatible with DD4hep Geant4Output2EDM4hepArrowStream writer

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <arrow/io/interfaces.h>
#include <arrow/ipc/reader.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <string>

class JEventSourcePODIOArrowStream : public JEventSource {

public:
  JEventSourcePODIOArrowStream(std::string resource_name, JApplication* app);

  virtual ~JEventSourcePODIOArrowStream();

  void Open() override;

  void Close() override;

  Result Emit(JEvent& event) override;

  static std::string GetDescription();

protected:
  std::shared_ptr<arrow::io::InputStream> m_input_stream;
  std::shared_ptr<arrow::ipc::RecordBatchStreamReader> m_arrow_reader;

  std::size_t m_events_read = 0;

  bool m_run_forever       = false;
  bool m_use_event_headers = true;

  std::shared_ptr<spdlog::logger> m_log;
};

template <> double JEventSourceGeneratorT<JEventSourcePODIOArrowStream>::CheckOpenable(std::string);
