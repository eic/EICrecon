#include "JEventProcessorManagedPODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Services/JComponentManager.h>
#include <JANA/Services/JParameterManager.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <podio/Writer.h>
#include <zmq.hpp>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "services/io/podio/JEventSourcePODIO.h"
#include "services/io/podio/JEventSourceManagedPODIO.h"
#include "services/log/Log_service.h"

JEventProcessorManagedPODIO::JEventProcessorManagedPODIO() : JEventProcessorPODIO() {
  SetTypeName(NAME_OF_THIS);

  // Override the default output file parameter since we'll manage it dynamically
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
  m_log = app->GetService<Log_service>()->logger("JEventProcessorManagedPODIO");

  m_log->info("Initializing managed PODIO processor with socket: {}", m_socket_path);

  // Initialize ZeroMQ
  try {
    m_zmq_context = std::make_unique<zmq::context_t>(1);
    m_zmq_socket = std::make_unique<zmq::socket_t>(*m_zmq_context, ZMQ_REP);
    
    // Remove existing socket file if it exists
    std::filesystem::remove(m_socket_path);
    
    // Bind to UNIX socket
    std::string bind_address = "ipc://" + m_socket_path;
    m_zmq_socket->bind(bind_address);
    
    m_log->info("ZeroMQ socket bound to: {}", bind_address);
    
    // Start listener thread
    m_listener_thread = std::make_unique<std::thread>(&JEventProcessorManagedPODIO::ListenForMessages, this);
    
  } catch (const std::exception& e) {
    throw std::runtime_error(fmt::format("Failed to initialize ZeroMQ: {}", e.what()));
  }

  // Don't call parent Init() since we'll manage the writer ourselves
}

void JEventProcessorManagedPODIO::ListenForMessages() {
  m_log->info("Started listening for messages on socket: {}", m_socket_path);
  
  while (!m_should_stop) {
    try {
      zmq::message_t request;
      
      // Poll for messages with timeout
      zmq::pollitem_t items[] = {{*m_zmq_socket, 0, ZMQ_POLLIN, 0}};
      int rc = zmq::poll(items, 1, std::chrono::milliseconds(1000));
      
      if (rc > 0 && (items[0].revents & ZMQ_POLLIN)) {
        auto result = m_zmq_socket->recv(request, zmq::recv_flags::dontwait);
        if (result) {
          std::string request_str(static_cast<char*>(request.data()), request.size());
          m_log->debug("Received message: {}", request_str);
          
          try {
            nlohmann::json request_json = nlohmann::json::parse(request_str);
            ProcessFileRequest(request_json);
          } catch (const std::exception& e) {
            m_log->error("Failed to parse JSON request: {}", e.what());
            nlohmann::json error_response = {
              {"status", "error"},
              {"message", fmt::format("Invalid JSON: {}", e.what())}
            };
            SendResponse(error_response);
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
      nlohmann::json error_response = {
        {"status", "error"},
        {"message", "Request must contain 'input_file' and 'output_file' fields"}
      };
      SendResponse(error_response);
      return;
    }
    
    std::string input_file = request["input_file"];
    std::string output_file = request["output_file"];
    
    m_log->info("Processing request: {} -> {}", input_file, output_file);
    
    // Check if input file exists
    if (!std::filesystem::exists(input_file)) {
      nlohmann::json error_response = {
        {"status", "error"},
        {"message", fmt::format("Input file does not exist: {}", input_file)}
      };
      SendResponse(error_response);
      return;
    }
    
    {
      std::lock_guard<std::mutex> lock(m_file_mutex);
      m_current_input_file = input_file;
      m_current_output_file = output_file;
      m_file_processing_active = true;
      m_events_processed = 0;
      m_pending_response = true;  // Mark that we have a pending response to send
    }
    
    // Open output file before notifying the source
    OpenOutputFile(output_file);
    
    // Signal the event source that a new file is available
    NotifySourceNewFile(input_file);
    
    m_log->info("Started processing file: {} -> {}", input_file, output_file);
    
    // Note: We don't send a response here. The response will be sent when processing completes
    // in CloseOutputFile()
    
  } catch (const std::exception& e) {
    m_log->error("Error processing file request: {}", e.what());
    nlohmann::json error_response = {
      {"status", "error"},
      {"message", fmt::format("Processing error: {}", e.what())}
    };
    SendResponse(error_response);
  }
}

void JEventProcessorManagedPODIO::SendResponse(const nlohmann::json& response) {
  try {
    std::string response_str = response.dump();
    zmq::message_t reply(response_str.size());
    memcpy(reply.data(), response_str.c_str(), response_str.size());
    m_zmq_socket->send(reply, zmq::send_flags::dontwait);
    m_log->debug("Sent response: {}", response_str);
  } catch (const std::exception& e) {
    m_log->error("Failed to send response: {}", e.what());
  }
}

void JEventProcessorManagedPODIO::OpenOutputFile(const std::string& output_file) {
  try {
    // Convert backend selection to lowercase for case-insensitive comparison
    std::string backend_lower = m_output_backend;
    std::transform(backend_lower.begin(), backend_lower.end(), backend_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    m_log->info("Opening output file: {} with backend: {}", output_file, backend_lower);

    // Create writer using podio::makeWriter
    m_writer = std::make_unique<podio::Writer>(podio::makeWriter(output_file, backend_lower));
    
  } catch (const std::exception& e) {
    throw std::runtime_error(
        fmt::format("Failed to create writer for file '{}' with backend '{}': {}", 
                   output_file, m_output_backend, e.what()));
  }
}

void JEventProcessorManagedPODIO::CloseOutputFile() {
  if (m_writer) {
    try {
      // Propagate non-event frames (same as parent class)
      auto* app = GetApplication();
      auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();
      for (auto* source : event_sources) {
        auto* podio_source = dynamic_cast<JEventSourcePODIO*>(source);
        if (podio_source == nullptr)
          continue;
        for (const auto& _category : podio_source->getAvailableCategories()) {
          std::string category{_category};
          if (category == "events")
            continue;
          std::size_t n = podio_source->getEntries(category);
          for (std::size_t i = 0; i < n; ++i) {
            m_writer->writeFrame(podio_source->getFrame(category, i), category);
          }
          m_log->info("Propagated {} '{}' frame(s) to output file", n, category);
        }
      }
      
      m_writer->finish();
      m_writer.reset();
      
      m_log->info("Closed output file: {}", m_current_output_file);
      
      // Send completion response only if we have a pending response
      if (m_pending_response) {
        nlohmann::json completion_response = {
          {"status", "completed"},
          {"input_file", m_current_input_file},
          {"output_file", m_current_output_file},
          {"events_processed", m_events_processed.load()}
        };
        SendResponse(completion_response);
        m_pending_response = false;
      }
      
    } catch (const std::exception& e) {
      m_log->error("Error closing output file: {}", e.what());
      if (m_pending_response) {
        nlohmann::json error_response = {
          {"status", "error"},
          {"message", fmt::format("Error closing file: {}", e.what())}
        };
        SendResponse(error_response);
        m_pending_response = false;
      }
    }
  }
}

void JEventProcessorManagedPODIO::Process(const std::shared_ptr<const JEvent>& event) {
  std::lock_guard<std::mutex> lock(m_file_mutex);
  
  if (!m_file_processing_active || !m_writer) {
    return; // No active file processing
  }
  
  // Call parent class implementation
  JEventProcessorPODIO::Process(event);
  
  m_events_processed++;
  
  // Check if current file processing is complete
  CheckFileCompletion();
}

void JEventProcessorManagedPODIO::Finish() {
  m_should_stop = true;
  
  if (m_file_processing_active) {
    CloseOutputFile();
    m_file_processing_active = false;
  }
  
  if (m_listener_thread && m_listener_thread->joinable()) {
    m_listener_thread->join();
  }
  
  // Clean up socket file
  std::filesystem::remove(m_socket_path);
  
  m_log->info("Managed PODIO processor finished");
}

void JEventProcessorManagedPODIO::NotifySourceNewFile(const std::string& input_file) {
  // Find the managed event source and notify it of the new file
  auto* app = GetApplication();
  auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();
  
  for (auto* source : event_sources) {
    auto* managed_source = dynamic_cast<JEventSourceManagedPODIO*>(source);
    if (managed_source != nullptr) {
      m_log->debug("Notifying managed source of new file: {}", input_file);
      managed_source->SetCurrentFile(input_file);
      break;
    }
  }
}

void JEventProcessorManagedPODIO::CheckFileCompletion() {
  if (m_file_processing_active && IsCurrentFileComplete()) {
    m_log->info("File processing completed, closing output file");
    CloseOutputFile();
    m_file_processing_active = false;
  }
}

bool JEventProcessorManagedPODIO::IsCurrentFileComplete() {
  auto* app = GetApplication();
  auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();
  
  for (auto* source : event_sources) {
    auto* managed_source = dynamic_cast<JEventSourceManagedPODIO*>(source);
    if (managed_source != nullptr) {
      return managed_source->IsFileProcessingComplete();
    }
  }
  return false;
}
