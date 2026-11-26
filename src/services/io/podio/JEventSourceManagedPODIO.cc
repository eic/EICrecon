#include "JEventSourceManagedPODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JEventSource.h>
#include <JANA/Utils/JTypeInfo.h>
#include <fmt/format.h>
#include <podio/Reader.h>
#include <spdlog/logger.h>
#include <exception>
#include <memory>
#include <utility>

#include "services/io/podio/JEventSourcePODIO.h"
#include "services/log/Log_service.h"

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
  m_closing = true;
  m_file_cv.notify_all();
}

JEventSourceManagedPODIO::Result JEventSourceManagedPODIO::Emit(JEvent& event) {
  std::unique_lock<std::mutex> lock(m_file_mutex);

  while (!m_file_available || !m_reader) {
    if (m_closing) {
      return Result::FailureFinished;
    }
    m_log->info("Waiting for the next file...");
    m_file_cv.wait(lock, [this] { return m_file_available.load() || m_closing.load(); });
  }

  if (m_closing) {
    return Result::FailureFinished;
  }

  // Check if we have emitted all events to process
  std::size_t events_emitted = m_next_entry - m_nskip;
  if (events_emitted >= m_nevents_to_process) {
    m_log->info("No more events available in current file, waiting for next file");
    m_file_processing_complete = true;
    m_file_available           = false;
    return Result::FailureTryAgain;
  }

  // Use parent class logic to read the event
  Result result = JEventSourcePODIO::Emit(event);

  // Check if we have now emitted all events to process
  events_emitted = m_next_entry - m_nskip;
  if (events_emitted >= m_nevents_to_process) {
    m_log->info("Finished reading all requested events from file: {}", m_current_input_file);
    m_file_processing_complete = true;
  }

  return result;
}

std::pair<JEventSourceManagedPODIO::Result, std::size_t>
JEventSourceManagedPODIO::Skip(JEvent& event, std::size_t events_to_skip) {
  // Per-request skipping is handled in SetCurrentFile(); keep JANA's generic
  // Emit()-based skipping for jana:nskip so the request bookkeeping stays consistent.
  return JEventSource::Skip(event, events_to_skip); // NOLINT(bugprone-parent-virtual-call)
}

std::string JEventSourceManagedPODIO::GetDescription() {
  return "Managed PODIO source (waits for external file requests)";
}

void JEventSourceManagedPODIO::SetCurrentFile(const std::string& input_file, uint64_t nskip,
                                              uint64_t nevents) {
  std::lock_guard<std::mutex> lock(m_file_mutex);

  m_current_input_file = input_file;
  m_nskip              = nskip;
  m_nevents            = nevents;

  m_file_processing_complete = false;

  try {
    m_log->info("Opening file for processing: {}", m_current_input_file);

    // Reset per-file state before opening the new file
    m_trust_event_header = true;

    // Use parent class method to open the file
    SetResourceName(m_current_input_file);
    JEventSourcePODIO::Open();

    m_event_entries = m_reader->getEntries(k_event_category);

    // Clamp nskip to file size
    if (m_nskip > m_event_entries) {
      m_log->warn("nskip ({}) exceeds events in file ({}), clamping to file size", m_nskip,
                  m_event_entries);
      m_nskip = m_event_entries;
    }

    // Compute number of events to process
    std::size_t available_after_skip = m_event_entries - m_nskip;
    if (m_nevents > 0 && m_nevents < available_after_skip) {
      m_nevents_to_process = m_nevents;
    } else {
      m_nevents_to_process = available_after_skip;
    }

    // Position read cursor past the skipped events
    m_next_entry = m_nskip;

    m_log->info("Opened PODIO file \"{}\" with {} events (nskip={}, nevents={}, to_process={})",
                m_current_input_file, m_event_entries, m_nskip,
                m_nevents == 0 ? std::string("all") : std::to_string(m_nevents),
                m_nevents_to_process);

  } catch (const std::exception& e) {
    m_log->error("Failed to open file {}: {}", m_current_input_file, e.what());
    throw;
  }

  m_file_available = true;
  m_file_cv.notify_all();
}

void JEventSourceManagedPODIO::ResetReader() {
  std::lock_guard<std::mutex> lock(m_file_mutex);
  m_reader.reset();
  m_file_available = false;
}
