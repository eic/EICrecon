#include "JEventSourceManagedPODIO.h"

#include <JANA/JApplication.h>
#include <JANA/Utils/JTypeInfo.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <podio/Reader.h>
#include <podio/podioVersion.h>
#include <spdlog/logger.h>
#include <exception>
#include <filesystem>
#include <memory>
#include <stdexcept>

#include "services/io/podio/JEventSourcePODIO.h"
#include "services/log/Log_service.h"

// Formatter for podio::version::Version
template <> struct fmt::formatter<podio::version::Version> : ostream_formatter {};

JEventSourceManagedPODIO::JEventSourceManagedPODIO(std::string resource_name, JApplication* app)
    : JEventSourcePODIO(resource_name, app) {
  SetTypeName(NAME_OF_THIS);

  m_log = GetApplication()->GetService<Log_service>()->logger("JEventSourceManagedPODIO");
}

JEventSourceManagedPODIO::~JEventSourceManagedPODIO() {}

void JEventSourceManagedPODIO::Open() {
  m_log->info("Opening managed PODIO source - waiting for file requests");
}

void JEventSourceManagedPODIO::Close() {
  m_log->info("Closing Managed Event Source");
  m_file_cv.notify_all();
}

JEventSourceManagedPODIO::Result JEventSourceManagedPODIO::Emit(JEvent& event) {
  std::unique_lock<std::mutex> lock(m_file_mutex);

  while (!m_file_available || !m_reader) {
    m_log->info("Waiting for the next file...");
    m_file_cv.wait(lock, [this] { return m_file_available.load(); });
  }

  // Check if we have events left to read
  if (Nevents_read >= Nevents_in_file) {
    m_log->info("No more events available in current file, waiting for next file");
    m_file_processing_complete = true;
    m_file_available           = false;
    m_reader.reset();
    return Result::FailureTryAgain;
  }

  // Use parent class logic to read the event
  Result result = JEventSourcePODIO::Emit(event);

  // Check if this was the last event
  if (Nevents_read >= Nevents_in_file) {
    m_log->info("Finished reading all events from file: {}", m_current_input_file);
    m_file_processing_complete = true;
  }

  return result;
}

std::string JEventSourceManagedPODIO::GetDescription() {
  return "Managed PODIO source (waits for external file requests)";
}

void JEventSourceManagedPODIO::SetCurrentFile(const std::string& input_file) {
  std::lock_guard<std::mutex> lock(m_file_mutex);

  m_current_input_file = input_file;

  m_file_processing_complete = false;

  try {
    m_log->info("Opening file for processing: {}", m_current_input_file);

    // Check if input file exists
    if (!std::filesystem::exists(m_current_input_file)) {
      throw std::runtime_error(fmt::format("Input file does not exist: {}", m_current_input_file));
    }

    // Use parent class method to open the file
    SetResourceName(m_current_input_file);
    JEventSourcePODIO::Open();

    Nevents_in_file = m_reader->getEntries("events");
    Nevents_read    = 0;

    m_log->info("Opened PODIO file \"{}\" with {} events", m_current_input_file, Nevents_in_file);

  } catch (const std::exception& e) {
    m_log->error("Failed to open file {}: {}", m_current_input_file, e.what());
    throw;
  }

  m_file_available = true;
  m_file_cv.notify_all();
}

template <>
double JEventSourceGeneratorT<JEventSourceManagedPODIO>::CheckOpenable(std::string resource_name) {
  // Only handle the magical constant "eicrecon://managed"
  if (resource_name == "eicrecon://managed") {
    return 1.0; // Highest priority for this specific resource name
  }
  return 0.0;
}
