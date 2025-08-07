// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
// This is a JANA event source that uses PODIO to read from a ROOT
// file created using the EDM4hep Data Model.
//

#include "JEventSource_DirectoryWatcher.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <optional>
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <podio/podioVersion.h>
#include <exception>
#include <memory>
#include <utility>

// These files are generated automatically by make_datamodel_glue.py
#include "services/io/podio/datamodel_glue.h"
#include "services/io/podio/datamodel_includes.h" // IWYU pragma: keep
#include "services/log/Log_service.h"

// Formatter for podio::version::Version
template <> struct fmt::formatter<podio::version::Version> : ostream_formatter {};

//------------------------------------------------------------------------------
// InsertingVisitor
//
/// This datamodel visitor will insert a PODIO collection into a JEvent.
/// This allows us to access the PODIO data through JEvent::Get and JEvent::GetCollection.
/// This makes it transparent to downstream factories whether the data was loaded from file, or calculated.
/// InsertingVisitor is called in GetEvent()
///
/// \param event             JANA JEvent to copy the data objects into
/// \param collection_name   name of the collection which will be used as the factory tag for these objects
//------------------------------------------------------------------------------
struct InsertingVisitor {
  JEvent& m_event;
  const std::string& m_collection_name;

  InsertingVisitor(JEvent& event, const std::string& collection_name)
      : m_event(event), m_collection_name(collection_name) {};

  template <typename T> void operator()(const T& collection) {

    using ContentsT = decltype(collection[0]);
    m_event.InsertCollectionAlreadyInFrame<ContentsT>(&collection, m_collection_name);
  }
};

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
JEventSource_DirectoryWatcher::JEventSource_DirectoryWatcher() {
  SetTypeName(NAME_OF_THIS); // Provide JANA with class name
  SetCallbackStyle(CallbackStyle::ExpertMode); // Use new, exception-free Emit() callback
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
JEventSource_DirectoryWatcher::~JEventSource_DirectoryWatcher() = default;

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
void JEventSource_DirectoryWatcher::Init() {
  m_log = GetApplication()->GetService<Log_service>()->logger("JEventSource_DirectoryWatcher");
}

//------------------------------------------------------------------------------
// Open
//
/// Open the root file and read in metadata.
//------------------------------------------------------------------------------
void JEventSource_DirectoryWatcher::Open() {
  m_directory_watcher.SetDirectoryToWatch(GetResourceName());
  m_directory_watcher.Update();
}

void JEventSource_DirectoryWatcher::OpenFile(std::string filename) {
  m_current_reader = std::make_unique<podio::ROOTReader>();
  try {
    m_current_reader->openFile(filename);
    auto version = m_current_reader->currentFileVersion();
    m_current_filename = filename;
    m_events_in_file = m_current_reader->getEntries("events");
    m_events_emitted = 0;

    m_log->info("Opened PODIO file: '{}'", filename);
    m_log->info("- version: file={} (executable={})", version, podio::version::build_version);
    m_log->info("- entries: {}", m_events_in_file);
  } 
  catch (std::exception& e) {
    m_log->info("Failure opening PODIO file: '{}'", filename);
    m_log->error("- Message: {}", e.what());
    m_directory_watcher.FinishFailure(filename);
    m_current_reader = nullptr;
  }
}

//------------------------------------------------------------------------------
// Close
//
/// Cleanly close the resource when JANA is terminated via Ctrl-C or jana:nevents
///
/// \param event
//------------------------------------------------------------------------------
void JEventSource_DirectoryWatcher::Close() {
  if (m_current_reader != nullptr) {
    m_current_reader = nullptr; // Destroys the last reader
    m_directory_watcher.FinishSuccess(m_current_filename);
    m_log->info("Closed PODIO file: '{}'", m_current_filename);
  }
}

//------------------------------------------------------------------------------
// GetEvent
//
/// Read next event from file and copy its objects into the given JEvent.
///
/// \param event
//------------------------------------------------------------------------------
JEventSource::Result JEventSource_DirectoryWatcher::Emit(JEvent& event) {

  while (m_current_reader == nullptr) {
    // If we don't have a current file, try to obtain a new one
    auto filename = m_directory_watcher.GetNextFilename();
    if (filename == std::nullopt) {
      return Result::FailureTryAgain;
    }
    // We have a file. Attempt to open it.
    OpenFile(*filename);
  }

  // At this point we know we have a valid file that contains at least one event
  auto frame_data = m_current_reader->readEntry("events", m_events_emitted);
  auto frame      = std::make_unique<podio::Frame>(std::move(frame_data));

  const auto& event_headers = frame->get<edm4hep::EventHeaderCollection>("EventHeader");
  event.SetEventNumber(event_headers.at(0).getEventNumber());
  event.SetRunNumber(event_headers.at(0).getRunNumber());

  VisitPodioCollection<InsertingVisitor> visit;
  for (const std::string& coll_name : frame->getAvailableCollections()) {
    const podio::CollectionBase* collection = frame->get(coll_name);
    InsertingVisitor visitor(event, coll_name);
    visit(visitor, *collection);
  }
  event.Insert(frame.release()); // Transfer ownership from unique_ptr to JFactoryT<podio::Frame>

  m_events_emitted += 1;
  if (m_events_emitted == m_events_in_file) {
    // Check if this was the last event. If so, close the file now so that the next Emit() call immediately looks for a new one
    m_current_reader = nullptr;
    m_directory_watcher.FinishSuccess(m_current_filename);
    m_log->info("Closed PODIO file: '{}'", m_current_filename);
  }
  return Result::Success;
}

//------------------------------------------------------------------------------
// GetDescription
//------------------------------------------------------------------------------
std::string JEventSource_DirectoryWatcher::GetDescription() {

  /// GetDescription() helps JANA explain to the user what is going on
  return "PODIO directory watcher";
}

//------------------------------------------------------------------------------
// CheckOpenable
//
/// Return a value from 0-1 indicating probability that this source will be
/// able to read this root file. Currently, it simply checks that the file
/// name is a directory.
/// \param resource_name name of root file to evaluate.
/// \return              value from 0-1 indicating confidence that this source can open the given file
//------------------------------------------------------------------------------
template <>
double JEventSourceGeneratorT<JEventSource_DirectoryWatcher>::CheckOpenable(std::string resource_name) {
  bool is_dir = std::filesystem::is_directory(resource_name);
  return (is_dir) ? 1.0 : 0.0;
}

