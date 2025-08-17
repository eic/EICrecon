
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <podio/ROOTWriter.h>
#include <spdlog/logger.h>
#include <memory>
#include <mutex>
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

  std::unique_ptr<podio::ROOTWriter> m_writer;
  std::mutex m_mutex;
  std::once_flag m_is_first_event;
  std::shared_ptr<spdlog::logger> m_log;
  bool m_output_include_collections_set = false;

  int m_events_per_file = 0;
  int m_events_written = 0;
  int m_file_suffix = 0;
  std::string m_temp_filename;
  std::string m_finished_filename;

  std::string m_output_file          = "podio_output.root";
  std::string m_output_file_copy_dir = "";
  std::set<std::string> m_output_collections;         // config. parameter
  std::set<std::string> m_output_exclude_collections; // config. parameter
  std::vector<std::string> m_collections_to_write;    // derived from above config. parameters
  std::vector<std::string> m_collections_to_print;
};
