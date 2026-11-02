#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <zmq.hpp>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

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
  void SendResponse(const nlohmann::json& response);  // listener thread only
  void QueueResponse(const nlohmann::json& response); // any thread; wakes listener
  void OpenOutputFile(const std::string& output_file);
  nlohmann::json CloseOutputFile();
  void NotifySourceNewFile(const std::string& input_file, uint64_t nskip, uint64_t nevents);
  bool IsCurrentFileComplete();
  std::size_t GetNeventsInCurrentFile();

  // ZeroMQ components
  std::unique_ptr<zmq::context_t> m_zmq_context;
  std::unique_ptr<zmq::socket_t> m_zmq_socket;
  std::string m_socket_path = "/tmp/eicrecon_managed.sock";

  std::unique_ptr<std::thread> m_listener_thread;
  std::atomic<bool> m_should_stop{false};

  // File management (protected by m_file_mutex)
  std::string m_current_input_file;
  std::string m_current_output_file;
  bool m_file_processing_active = false;
  std::mutex m_file_mutex;
  // Event counting for current file
  std::atomic<std::size_t> m_events_processed{0};

  // Response queue (protected by m_file_mutex; m_response_cv wakes the listener)
  std::optional<nlohmann::json> m_queued_response;
  std::condition_variable m_response_cv;

  // True between the listener's recv() and its matching send() (ZMQ_REP protocol)
  bool m_awaiting_reply = false;
};
