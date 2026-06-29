#include "JEventProcessorManagedPODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/Services/JComponentManager.h>
#include <JANA/Utils/JTypeInfo.h>
#include <errno.h>
#include <fmt/format.h>
#include <nlohmann/detail/json_ref.hpp>
#include <nlohmann/json.hpp>
#include <podio/Writer.h>
#include <spdlog/logger.h>
#include <zmq.h>
#include <zmq.hpp>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "services/io/podio/JEventProcessorPODIO.h"
#include "services/io/podio/JEventSourceManagedPODIO.h"
#include "services/log/Log_service.h"

JEventProcessorManagedPODIO::JEventProcessorManagedPODIO() : JEventProcessorPODIO() {
  SetTypeName(NAME_OF_THIS);

  japp->SetDefaultParameter("podio:managed_socket_path", m_socket_path,
                            "UNIX socket path for managed PODIO processing");
}

JEventProcessorManagedPODIO::~JEventProcessorManagedPODIO() {
  m_should_stop = true;
  if (m_listener_thread && m_listener_thread->joinable()) {
    m_listener_thread->join();
  }
}

void JEventProcessorManagedPODIO::Init() {
  auto* app = GetApplication();
  m_log     = app->GetService<Log_service>()->logger("JEventProcessorManagedPODIO");

  m_log->info("Initializing managed PODIO processor with socket: {}", m_socket_path);

  // Initialize ZeroMQ
  try {
    m_zmq_context = std::make_unique<zmq::context_t>(1);
    m_zmq_socket  = std::make_unique<zmq::socket_t>(*m_zmq_context, ZMQ_REP);

    // Remove existing socket file if it exists and is actually a socket
    if (std::filesystem::exists(m_socket_path)) {
      if (std::filesystem::is_socket(m_socket_path)) {
        std::filesystem::remove(m_socket_path);
        m_log->debug("Removed existing socket file: {}", m_socket_path);
      } else {
        throw std::runtime_error(fmt::format("Path exists but is not a socket: {}", m_socket_path));
      }
    }

    // Bind to UNIX socket
    std::string bind_address = "ipc://" + m_socket_path;
    m_zmq_socket->bind(bind_address);

    m_log->info("ZeroMQ socket bound to: {}", bind_address);

    // Start listener thread
    m_listener_thread =
        std::make_unique<std::thread>(&JEventProcessorManagedPODIO::ListenForMessages, this);

  } catch (const std::exception& e) {
    throw std::runtime_error(fmt::format("Failed to initialize ZeroMQ: {}", e.what()));
  }

  // Don't call parent Init() since we'll manage the writer ourselves
}

void JEventProcessorManagedPODIO::ListenForMessages() {
  m_log->info("Started listening for messages on socket: {}", m_socket_path);

  while (!m_should_stop) {
    try {
      // Drain any queued response deposited by the JANA thread.
      {
        std::lock_guard<std::mutex> lock(m_file_mutex);
        if (m_queued_response.has_value()) {
          nlohmann::json response = std::move(*m_queued_response);
          m_queued_response.reset();
          SendResponse(response);
          m_awaiting_reply = false;
          continue;
        }
      }

      // While the JANA thread is still working, wait instead of recv()ing
      // (ZMQ_REP requires strict recv-send alternation).
      if (m_awaiting_reply) {
        std::unique_lock<std::mutex> lock(m_file_mutex);
        m_response_cv.wait_for(lock, std::chrono::milliseconds(100), [this] {
          return m_queued_response.has_value() || m_should_stop.load();
        });
        continue;
      }

      // Poll for messages with timeout
      zmq::pollitem_t items[] = {{*m_zmq_socket, 0, ZMQ_POLLIN, 0}};
      int rc                  = zmq::poll(items, 1, std::chrono::milliseconds(1000));

      if (rc > 0 && (items[0].revents & ZMQ_POLLIN)) {
        zmq::message_t request;
        auto result = m_zmq_socket->recv(request, zmq::recv_flags::dontwait);
        if (result) {
          std::string request_str(static_cast<char*>(request.data()), request.size());
          m_log->debug("Received message: {}", request_str);

          try {
            nlohmann::json request_json = nlohmann::json::parse(request_str);
            ProcessFileRequest(request_json);
          } catch (const std::exception& e) {
            m_log->error("Failed to parse JSON request: {}", e.what());
            SendResponse(
                {{"status", "error"}, {"message", fmt::format("Invalid JSON: {}", e.what())}});
          }
        }
      }
    } catch (const zmq::error_t& e) {
      if (e.num() != EAGAIN && e.num() != EINTR) {
        m_log->error("ZeroMQ error in listener: {}", e.what());
      }
    } catch (const std::exception& e) {
      m_log->error("Unexpected error in listener: {}", e.what());
    }
  }

  m_log->info("Message listener thread stopped");
}

void JEventProcessorManagedPODIO::ProcessFileRequest(const nlohmann::json& request) {
  try {
    if (!request.contains("input_file") || !request.contains("output_file")) {
      SendResponse({{"status", "error"},
                    {"message", "Request must contain 'input_file' and 'output_file' fields"}});
      return;
    }

    std::string input_file  = request["input_file"];
    std::string output_file = request["output_file"];

    // Extract optional nskip and nevents parameters (default to 0 = process all)
    uint64_t nskip   = request.value("nskip", uint64_t{0});
    uint64_t nevents = request.value("nevents", uint64_t{0});

    m_log->info("Processing request: {} -> {} (nskip={}, nevents={})", input_file, output_file,
                nskip, nevents == 0 ? std::string("0 [all]") : std::to_string(nevents));

    {
      std::lock_guard<std::mutex> lock(m_file_mutex);
      m_current_input_file  = input_file;
      m_current_output_file = output_file;
      m_events_processed    = 0;

      // Reset per-file writer state and open the output file while holding
      // m_file_mutex so that no JANA worker thread can observe
      // m_file_processing_active==true before m_writer is fully initialised.
      m_collections_to_write.clear();
      std::destroy_at(&m_is_first_event);
      std::construct_at(&m_is_first_event);
      OpenOutputFile(output_file);

      // Signal readiness only after the writer is in place.
      m_file_processing_active = true;
    }

    // Signal the event source that a new file is available.
    try {
      NotifySourceNewFile(input_file, nskip, nevents);
    } catch (...) {
      std::lock_guard<std::mutex> lock(m_file_mutex);
      m_file_processing_active = false;
      m_writer.reset(); // finalizes and closes the (empty) output file
      throw;
    }

    m_log->info("Started processing file: {} -> {}", input_file, output_file);

    // Zero-event files must be completed here because JANA will never call
    // Process(), so the completion check there would never run.
    if (GetNeventsInCurrentFile() == 0) {
      m_log->info("File has zero events, completing immediately");
      nlohmann::json response = CloseOutputFile();
      {
        std::lock_guard<std::mutex> lock(m_file_mutex);
        m_file_processing_active = false;
      }
      SendResponse(response);
      return;
    }

    // Reply will arrive later via QueueResponse() from the JANA thread.
    m_awaiting_reply = true;

  } catch (const std::exception& e) {
    m_log->error("Error processing file request: {}", e.what());
    SendResponse({{"status", "error"}, {"message", fmt::format("Processing error: {}", e.what())}});
  }
}

void JEventProcessorManagedPODIO::SendResponse(const nlohmann::json& response) {
  try {
    std::string response_str = response.dump();
    zmq::message_t reply(response_str.size());
    memcpy(reply.data(), response_str.c_str(), response_str.size());
    auto sent = m_zmq_socket->send(reply, zmq::send_flags::none);
    if (!sent) {
      throw std::runtime_error("ZeroMQ send failed");
    }
    m_log->debug("Sent response: {}", response_str);
  } catch (const std::exception& e) {
    m_log->error("Failed to send response: {}", e.what());
  }
}

void JEventProcessorManagedPODIO::QueueResponse(const nlohmann::json& response) {
  {
    std::lock_guard<std::mutex> lock(m_file_mutex);
    m_queued_response = response;
  }
  m_response_cv.notify_one();
}

void JEventProcessorManagedPODIO::OpenOutputFile(const std::string& output_file) {
  std::string backend_lower = m_output_backend;
  std::transform(backend_lower.begin(), backend_lower.end(), backend_lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  m_log->info("Opening output file: {} with backend: {}", output_file, backend_lower);

  try {
    m_writer = std::make_unique<podio::Writer>(podio::makeWriter(output_file, backend_lower));
  } catch (const std::exception& e) {
    throw std::runtime_error(
        fmt::format("Failed to create writer for file '{}' with backend '{}': {}", output_file,
                    m_output_backend, e.what()));
  }
}

nlohmann::json JEventProcessorManagedPODIO::CloseOutputFile() {
  if (!m_writer) {
    return {{"status", "error"}, {"message", "No active writer to close"}};
  }

  try {
    // Propagate non-"events" frames (e.g. "runs", "metadata") to the output.
    PropagateNonEventCategories();

    // Release the reader so it doesn't hold the input file open until the
    // next SetCurrentFile() call.
    auto* app          = GetApplication();
    auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();
    for (auto* source : event_sources) {
      auto* managed_source = dynamic_cast<JEventSourceManagedPODIO*>(source);
      if (managed_source != nullptr) {
        managed_source->ResetReader();
      }
    }

    m_writer->finish();
    m_writer.reset();

    std::string current_output_file;
    std::string current_input_file;
    {
      std::lock_guard<std::mutex> lock(m_file_mutex);
      current_output_file = m_current_output_file;
      current_input_file  = m_current_input_file;
    }

    m_log->info("Closed output file: {}", current_output_file);

    return {{"status", "completed"},
            {"input_file", current_input_file},
            {"output_file", current_output_file},
            {"events_processed", m_events_processed.load()}};

  } catch (const std::exception& e) {
    m_log->error("Error closing output file: {}", e.what());
    m_writer.reset();
    return {{"status", "error"}, {"message", fmt::format("Error closing file: {}", e.what())}};
  }
}

void JEventProcessorManagedPODIO::Process(const std::shared_ptr<const JEvent>& event) {
  bool should_close = false;
  {
    std::lock_guard<std::mutex> lock(m_file_mutex);

    if (!m_file_processing_active || !m_writer) {
      return; // No active file processing
    }

    // Call parent class implementation
    JEventProcessorPODIO::Process(event);

    m_events_processed++;

    // Check completion while we still hold the lock so that concurrent
    // Process() threads cannot race into a second close.
    if (IsCurrentFileComplete()) {
      m_file_processing_active = false;
      should_close             = true;
    }
  }

  // CloseOutputFile() acquires m_file_mutex internally, so call it outside our lock.
  if (should_close) {
    m_log->info("File processing completed, closing output file");
    nlohmann::json response = CloseOutputFile();
    QueueResponse(response);
  }
}

void JEventProcessorManagedPODIO::Finish() {
  m_should_stop = true;

  bool should_close_file = false;
  {
    std::lock_guard<std::mutex> lock(m_file_mutex);
    should_close_file = m_file_processing_active;
    // Clear the flag under the same lock so that a concurrent Process()
    // thread cannot also see it as true and race into a second close.
    m_file_processing_active = false;
  }

  if (should_close_file) {
    CloseOutputFile();
  }

  if (m_listener_thread && m_listener_thread->joinable()) {
    m_listener_thread->join();
  }

  // Clean up socket file if it exists and is a socket
  if (std::filesystem::exists(m_socket_path) && std::filesystem::is_socket(m_socket_path)) {
    std::filesystem::remove(m_socket_path);
    m_log->debug("Cleaned up socket file: {}", m_socket_path);
  }

  m_log->info("Managed PODIO processor finished");
}

void JEventProcessorManagedPODIO::NotifySourceNewFile(const std::string& input_file, uint64_t nskip,
                                                      uint64_t nevents) {
  // Find the managed event source and notify it of the new file
  auto* app          = GetApplication();
  auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();

  for (auto* source : event_sources) {
    auto* managed_source = dynamic_cast<JEventSourceManagedPODIO*>(source);
    if (managed_source != nullptr) {
      m_log->debug("Notifying managed source of new file: {}", input_file);
      managed_source->SetCurrentFile(input_file, nskip, nevents);
      break;
    }
  }
}

bool JEventProcessorManagedPODIO::IsCurrentFileComplete() {
  auto* app          = GetApplication();
  auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();

  for (auto* source : event_sources) {
    auto* managed_source = dynamic_cast<JEventSourceManagedPODIO*>(source);
    if (managed_source != nullptr) {
      return managed_source->IsFileProcessingComplete();
    }
  }
  return false;
}

std::size_t JEventProcessorManagedPODIO::GetNeventsInCurrentFile() {
  auto* app          = GetApplication();
  auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();

  for (auto* source : event_sources) {
    auto* managed_source = dynamic_cast<JEventSourceManagedPODIO*>(source);
    if (managed_source != nullptr) {
      return managed_source->GetNeventsInFile();
    }
  }
  return 0;
}
