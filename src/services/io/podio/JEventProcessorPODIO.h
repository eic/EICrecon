
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <podio/Writer.h>
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

protected:
  /// Propagate all non-"events" frames from any JEventSourcePODIO input source(s)
  /// to the output file.
  void PropagateNonEventCategories();

  /// Append a "runs" frame that records this EICrecon invocation: producer,
  /// processing stage, versions, inputs, and the full JANA parameter set.
  /// Must be called after PropagateNonEventCategories().
  void WriteRunFrame();

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

  // State gathered from propagated input "runs" frames
  int m_max_input_processing_stage = 0;
  std::vector<std::string> m_input_runs_collections;
};
