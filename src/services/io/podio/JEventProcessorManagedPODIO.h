#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <nlohmann/json.hpp>
#include <podio/Writer.h>
#include <spdlog/logger.h>
#include <zmq.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "JEventProcessorPODIO.h"

class JEventProcessorManagedPODIO : public JEventProcessorPODIO {

public:
  JEventProcessorManagedPODIO();
  virtual ~JEventProcessorManagedPODIO();

  void Init() override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void Finish() override;

private:
  void ListenForMessages();
  void ProcessFileRequest(const nlohmann::json& request);
  void SendResponse(const nlohmann::json& response);
  void OpenOutputFile(const std::string& output_file);
  void CloseOutputFile();
  void NotifySourceNewFile(const std::string& input_file);
  void CheckFileCompletion();
  bool IsCurrentFileComplete();

  // ZeroMQ components
  std::unique_ptr<zmq::context_t> m_zmq_context;
  std::unique_ptr<zmq::socket_t> m_zmq_socket;
  std::string m_socket_path = "/tmp/eicrecon_managed.sock";
  
  // Threading
  std::unique_ptr<std::thread> m_listener_thread;
  std::atomic<bool> m_should_stop{false};
  
  // File management
  std::string m_current_input_file;
  std::string m_current_output_file;
  bool m_file_processing_active = false;
  std::mutex m_file_mutex;
  
  // Event counting for current file
  std::atomic<size_t> m_events_processed{0};
  
  // Response management
  bool m_pending_response = false;
};
