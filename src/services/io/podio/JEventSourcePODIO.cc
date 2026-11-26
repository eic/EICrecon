// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#include "JEventSourcePODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TFile.h>
#include <edm4hep/EventHeaderCollection.h>
#include <podio/CollectionBase.h>
#include <podio/podioVersion.h>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "services/io/podio/datamodel_glue.h" // IWYU pragma: keep
#include "services/log/Log_service.h"

namespace {

// Registers a collection that is owned by the event's podio::Frame as a
// JPodioDatabundle, dispatching on the concrete collection type.
class FrameCollectionPublisher {
public:
  FrameCollectionPublisher(JEvent* event, std::string name)
      : m_event(event), m_name(std::move(name)) {}

  template <typename CollectionT> void operator()(const CollectionT& collection) {
    using ElementT = typename CollectionT::value_type;
    m_event->InsertCollectionAlreadyInFrame<ElementT>(&collection, m_name);
  }

private:
  JEvent* m_event;
  std::string m_name;
};

std::string to_string(const podio::version::Version& v) {
  std::ostringstream os;
  os << v;
  return os.str();
}

} // namespace

// ---------------------------------------------------------------------------
// JEventSourcePODIO
// ---------------------------------------------------------------------------

JEventSourcePODIO::JEventSourcePODIO(std::string resource_name, JApplication* app)
    : JEventSource(std::move(resource_name), app) {
  SetTypeName(NAME_OF_THIS);
  SetCallbackStyle(CallbackStyle::ExpertMode);
  EnableProcessParallel();

  m_log = app->GetService<Log_service>()->logger("JEventSourcePODIO");

  // Registered eagerly (rather than as JComponent::Parameter members) so that
  // they are visible to `eicrecon --configs` and to eicrecon_cli before the
  // topology is built.
  app->SetDefaultParameter("podio:run_forever", m_run_forever,
                           "set to true to recycle through events continuously");
  app->SetDefaultParameter("podio:print_type_table", m_print_type_table,
                           "Print list of collection names and their types");
}

std::string JEventSourcePODIO::GetDescription() { return "PODIO ROOT file"; }

void JEventSourcePODIO::Open() {
  const std::string& path = GetResourceName();

  try {
    m_reader = std::make_unique<podio::Reader>(podio::makeReader(path));
  } catch (const std::exception& e) {
    m_log->error("{}", e.what());
    throw JException(std::format("Problem opening file \"{}\": {}", path, e.what()));
  }

  const auto file_version = m_reader->currentFileVersion();
  const auto exe_version  = podio::version::build_version;
  m_log->info("PODIO version: file={} (executable={})", to_string(file_version),
              to_string(exe_version));
  // Files written by a newer podio minor release may use schema features that
  // this build does not understand. Reading is still attempted.
  if (std::tie(file_version.major, file_version.minor) >
      std::tie(exe_version.major, exe_version.minor)) {
    m_log->warn("Input file was written with a newer podio ({}) than this executable ({})",
                to_string(file_version), to_string(exe_version));
  }

  m_event_entries      = m_reader->getEntries(k_event_category);
  m_next_entry         = 0;
  m_trust_event_header = true;
  m_log->info("Opened PODIO file \"{}\" with {} events (format auto-detected)", path,
              m_event_entries);

  if (m_print_type_table) {
    PrintCollectionTypeTable();
  }
}

void JEventSourcePODIO::Close() {
  // The reader is deliberately kept alive: JEventProcessorPODIO::Finish() reads
  // the non-event categories through getFrame() after this source has closed.
  m_log->debug("Finished reading events from \"{}\"", GetResourceName());
}

bool JEventSourcePODIO::rewindIfExhausted() {
  if (m_next_entry < m_event_entries) {
    return true;
  }
  if (!m_run_forever || m_event_entries == 0) {
    return false;
  }
  m_next_entry = 0;
  return true;
}

JEventSource::Result JEventSourcePODIO::Emit(JEvent& event) {
  // Emit() is serialized by JANA, so the cursor needs no locking.
  if (!rewindIfExhausted()) {
    return Result::FailureFinished;
  }

  auto frame = std::make_unique<podio::Frame>(m_reader->readFrame(k_event_category, m_next_entry));
  event.Insert(frame.get());
  const podio::Frame& inserted = *frame.release(); // owned by the JEvent from here on

  applyEventHeader(inserted, event);

  // Collections are published (and thereby unpacked) in ProcessParallel().
  ++m_next_entry;
  return Result::Success;
}

void JEventSourcePODIO::ProcessParallel(JEvent& event) const {
  // Runs concurrently for different events and only touches this event's
  // frame. Frame::get() unpacks the collection buffers on first access, so the
  // deserialization cost is spread over the worker threads instead of being
  // paid inside the serialized Emit().
  const auto* frame = event.GetSingle<podio::Frame>();
  for (const std::string& name : frame->getAvailableCollections()) {
    FrameCollectionPublisher publisher(&event, name);
    VisitPodioCollection<FrameCollectionPublisher>{}(publisher, *frame->get(name));
  }
}

std::pair<JEventSource::Result, std::size_t> JEventSourcePODIO::Skip(JEvent& /*event*/,
                                                                     std::size_t events_to_skip) {
  // Entries are addressed by index, so skipping only moves the cursor and
  // avoids reading and unpacking frames that would be discarded.
  while (events_to_skip > 0) {
    if (!rewindIfExhausted()) {
      return {Result::FailureFinished, events_to_skip};
    }
    const std::size_t step = std::min(events_to_skip, m_event_entries - m_next_entry);
    m_next_entry += step;
    events_to_skip -= step;
  }
  return {Result::Success, 0};
}

void JEventSourcePODIO::applyEventHeader(const podio::Frame& frame, JEvent& event) {
  if (!m_trust_event_header) {
    return;
  }
  const podio::CollectionBase* collection = frame.get("EventHeader");
  const auto* headers = dynamic_cast<const edm4hep::EventHeaderCollection*>(collection);
  if (headers == nullptr || headers->size() != 1) {
    std::string problem;
    if (collection == nullptr) {
      problem = "has no EventHeader collection";
    } else if (headers == nullptr) {
      problem = std::format("has an EventHeader collection of type {} instead of {}",
                            collection->getTypeName(), edm4hep::EventHeaderCollection::typeName);
    } else {
      problem = std::format("has {} EventHeader items, but 1 expected", headers->size());
    }
    m_log->warn("Entry {} {}. Will not use event and run numbers from header", m_next_entry,
                problem);
    m_trust_event_header = false;
    return;
  }
  const auto header = (*headers)[0];
  event.SetEventNumber(header.getEventNumber());
  event.SetRunNumber(header.getRunNumber());
}

std::vector<std::string_view> JEventSourcePODIO::getAvailableCategories() const {
  return m_reader->getAvailableCategories();
}

std::size_t JEventSourcePODIO::getEntries(const std::string& category) const {
  return m_reader->getEntries(category);
}

podio::Frame JEventSourcePODIO::getFrame(const std::string& category, std::size_t index) const {
  return m_reader->readFrame(category, index);
}

void JEventSourcePODIO::PrintCollectionTypeTable() {
  if (!m_reader || m_event_entries == 0) {
    m_log->warn("No events available in \"{}\"; cannot list collections", GetResourceName());
    return;
  }

  // Collection content is the same for every entry, so the first one suffices.
  const podio::Frame frame = m_reader->readFrame(k_event_category, 0);

  std::map<std::string, std::string> types; // sorted by collection name
  for (const std::string& name : frame.getAvailableCollections()) {
    types.emplace(name, std::string(frame.get(name)->getTypeName()));
  }

  const std::string name_header = "Collection Name";
  const std::string type_header = "Data Type";
  std::size_t name_width        = name_header.size();
  std::size_t type_width        = type_header.size();
  for (const auto& [name, type] : types) {
    name_width = std::max(name_width, name.size());
    type_width = std::max(type_width, type.size());
  }

  std::cout << "\nAvailable Collections\n\n";
  std::cout << std::format("{:<{}}  {}\n", name_header, name_width, type_header);
  std::cout << std::format("{}  {}\n", std::string(name_width, '-'), std::string(type_width, '-'));
  for (const auto& [name, type] : types) {
    std::cout << std::format("{:<{}}  {}\n", name, name_width, type);
  }
  std::cout << std::endl;
}

// ---------------------------------------------------------------------------
// JEventSourcePODIOGenerator
// ---------------------------------------------------------------------------

std::string JEventSourcePODIOGenerator::GetType() const {
  return JTypeInfo::demangle<JEventSourcePODIO>();
}

std::string JEventSourcePODIOGenerator::GetDescription() const {
  return JEventSourcePODIO::GetDescription();
}

JEventSource* JEventSourcePODIOGenerator::MakeJEventSource(std::string resource_name) {
  auto* source = new JEventSourcePODIO(std::move(resource_name), mApplication);
  if (GetLevel() != JEventLevel::None) {
    source->SetLevel(GetLevel());
  }
  source->SetPluginName(mPluginName);
  return source;
}

double JEventSourcePODIOGenerator::CheckOpenable(std::string resource_name) {
  // Only ROOT-based podio files are handled here.
  if (resource_name.find(".root") == std::string::npos) {
    return 0.0;
  }

  // Inspect the file with ROOT before handing it to podio, which does not
  // fail gracefully on non-podio ROOT files.
  const std::unique_ptr<TFile> file{TFile::Open(resource_name.c_str())};
  if (!file || file->IsZombie()) {
    return 0.0;
  }

  // Both the TTree and the RNTuple backends store their metadata under this
  // key. GetKey() is used instead of Get() so that the check does not depend on
  // the stored object deriving from TObject (RNTuple does not).
  if (file->GetKey("podio_metadata") == nullptr) {
    return 0.0;
  }
  return 0.03;
}
