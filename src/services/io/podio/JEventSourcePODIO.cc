// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
// This is a JANA event source that uses PODIO to read from a ROOT
// file created using the EDM4hep Data Model.

#include "JEventSourcePODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TFile.h>
#include <TObject.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <podio/podioVersion.h>
#include <algorithm>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "services/io/podio/datamodel_glue_compat.h"
#include "services/io/podio/datamodel_includes_compat.h" // IWYU pragma: keep
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
  // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members): Lifetime of referenced objects is guaranteed beyond visitor lifetime in this pattern
  JEvent& m_event;
  const std::string& m_collection_name;
  // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

  InsertingVisitor(JEvent& event, const std::string& collection_name)
      : m_event(event), m_collection_name(collection_name) {};

  template <typename T> void operator()(const T& collection) {

    using ContentsT = decltype(collection[0]);
    m_event.InsertCollectionAlreadyInFrame<ContentsT>(&collection, m_collection_name);
  }
};

//------------------------------------------------------------------------------
// Constructor
//
///
/// \param resource_name  Name of root file to open (n.b. file is not opened until Open() is called)
/// \param app            JApplication
//------------------------------------------------------------------------------
JEventSourcePODIO::JEventSourcePODIO(std::string resource_name, JApplication* app)
    : JEventSource(resource_name, app) {
  SetTypeName(NAME_OF_THIS); // Provide JANA with class name
#if JANA_NEW_CALLBACK_STYLE
  SetCallbackStyle(CallbackStyle::ExpertMode); // Use new, exception-free Emit() callback
#endif

  // Get Logger
  m_log = GetApplication()->GetService<Log_service>()->logger("JEventSourcePODIO");

  // Tell JANA that we want it to call the FinishEvent() method.
  // EnableFinishEvent();

  // Allow user to specify to recycle events forever
  GetApplication()->SetDefaultParameter("podio:run_forever", m_run_forever,
                                        "set to true to recycle through events continuously");

  bool print_type_table = false;
  GetApplication()->SetDefaultParameter("podio:print_type_table", print_type_table,
                                        "Print list of collection names and their types");

  // Hopefully we won't need to reimplement background event merging. Using podio frames, it looks like we would
  // have to do a deep copy of all data in order to insert it into the same frame, which would probably be
  // quite inefficient.
  /*
    std::string background_filename;
    GetApplication()->SetDefaultParameter(
            "podio:background_filename",
            background_filename,
            "Name of file containing background events to merge in (default is not to merge any background)"
            );

    int num_background_events=1;
    GetApplication()->SetDefaultParameter(
            "podio:num_background_events",
            num_background_events,
            "Number of background events to add to every primary event."
    );
    */
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
JEventSourcePODIO::~JEventSourcePODIO() {
  m_log->info("Closing Event Source for {}", GetResourceName());
}

//------------------------------------------------------------------------------
// Open
//
/// Open the root file and read in metadata.
//------------------------------------------------------------------------------
void JEventSourcePODIO::Open() {

  bool print_type_table = GetApplication()->GetParameterValue<bool>("podio:print_type_table");
  // std::string background_filename = GetApplication()->GetParameterValue<std::string>("podio:background_filename");;
  // int num_background_events = GetApplication()->GetParameterValue<int>("podio:num_background_events");;

  // Open primary events file
  try {

    m_reader.openFile(GetResourceName());

    auto version          = m_reader.currentFileVersion();
    bool version_mismatch = version.major > podio::version::build_version.major;
    version_mismatch |= (version.major == podio::version::build_version.major) &&
                        (version.minor > podio::version::build_version.minor);
    if (version_mismatch) {
      std::stringstream ss;
      ss << "Mismatch in PODIO versions! " << version << " > " << podio::version::build_version;
      // FIXME: The podio ROOTReader is somehow failing to read in the correct version numbers from the file
      //            throw JException(ss.str());
    }

    m_log->info("PODIO version: file={} (executable={})", version, podio::version::build_version);

    Nevents_in_file = m_reader.getEntries("events");
    m_log->info("Opened PODIO Frame file \"{}\" with {} events", GetResourceName(),
                Nevents_in_file);

    if (print_type_table) {
      PrintCollectionTypeTable();
    }

  } catch (std::exception& e) {
    m_log->error(e.what());
    throw JException(fmt::format("Problem opening file \"{}\"", GetResourceName()));
  }
}

//------------------------------------------------------------------------------
// Close
//
/// Cleanly close the resource when JANA is terminated via Ctrl-C or jana:nevents
///
/// \param event
//------------------------------------------------------------------------------
void JEventSourcePODIO::Close() {
  // m_reader.close();
  // TODO: ROOTReader does not appear to have a close() method.
}

//------------------------------------------------------------------------------
// GetEvent
//
/// Read next event from file and copy its objects into the given JEvent.
///
/// \param event
//------------------------------------------------------------------------------
#if JANA_NEW_CALLBACK_STYLE
JEventSourcePODIO::Result JEventSourcePODIO::Emit(JEvent& event) {
#else
void JEventSourcePODIO::GetEvent(std::shared_ptr<JEvent> _event) {
  auto& event = *_event;
#endif

  /// Calls to GetEvent are synchronized with each other, which means they can
  /// read and write state on the JEventSource without causing race conditions.

  // Check if we have exhausted events from file
  if (Nevents_read >= Nevents_in_file) {
    if (m_run_forever) {
      Nevents_read = 0;
    } else {
#if JANA_NEW_CALLBACK_STYLE
      return Result::FailureFinished;
#else
      throw RETURN_STATUS::kNO_MORE_EVENTS;
#endif
    }
  }

  auto frame_data = m_reader.readEntry("events", Nevents_read);
  auto frame      = std::make_unique<podio::Frame>(std::move(frame_data));

  if (m_use_event_headers) {
    const auto& event_headers = frame->get<edm4hep::EventHeaderCollection>("EventHeader");
    if (event_headers.size() != 1) {
      m_log->warn("Missing or bad event headers: Entry {} contains {} items, but 1 expected. Will "
                  "not use event and run numbers from header",
                  Nevents_read, event_headers.size());
      m_use_event_headers = false;
    } else {
      event.SetEventNumber(event_headers[0].getEventNumber());
      event.SetRunNumber(event_headers[0].getRunNumber());
    }
  }

  // Insert contents odf frame into JFactories
  VisitPodioCollection<InsertingVisitor> visit;
  for (const std::string& coll_name : frame->getAvailableCollections()) {
    const podio::CollectionBase* collection = frame->get(coll_name);
    InsertingVisitor visitor(event, coll_name);
    visit(visitor, *collection);
  }

  event.Insert(frame.release()); // Transfer ownership from unique_ptr to JFactoryT<podio::Frame>
  Nevents_read += 1;
#if JANA_NEW_CALLBACK_STYLE
  return Result::Success;
#endif
}

//------------------------------------------------------------------------------
// GetDescription
//------------------------------------------------------------------------------
std::string JEventSourcePODIO::GetDescription() {

  /// GetDescription() helps JANA explain to the user what is going on
  return "PODIO root file (Frames, podio >= v0.16.3)";
}

//------------------------------------------------------------------------------
// CheckOpenable
//
/// Return a value from 0-1 indicating probability that this source will be
/// able to read this root file. Currently, it simply checks that the file
/// name contains the string ".root" and if does, returns a small number (0.02).
/// This will need to be made more sophisticated if the alternative root file
/// formats need to be supported by other event sources.
///
/// \param resource_name name of root file to evaluate.
/// \return              value from 0-1 indicating confidence that this source can open the given file
//------------------------------------------------------------------------------
template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string resource_name) {

  // PODIO Frame reader gets slightly higher precedence than PODIO Legacy reader, but only if the file
  // contains a 'podio_metadata' TTree. If the file doesn't exist, this will return 0. The "file not found"
  // error will hopefully be generated by the PODIO legacy reader instead.
  if (resource_name.find(".root") == std::string::npos) {
    return 0.0;
  }

  // PODIO FrameReader segfaults on legacy input files, so we use ROOT to validate beforehand. Of course,
  // we can't validate if ROOT can't read the file.
  std::unique_ptr<TFile> file = std::unique_ptr<TFile>{TFile::Open(resource_name.c_str())};
  if (!file || file->IsZombie()) {
    return 0.0;
  }

  // We test the format the same way that PODIO's python API does. See python/podio/reading.py
  TObject* tree = file->Get("podio_metadata");
  if (tree == nullptr) {
    return 0.0;
  }
  return 0.03;
}

//------------------------------------------------------------------------------
// PrintCollectionTypeTable
//
/// Print the list of collection names from the currently open file along
/// with their types. This will be called automatically when the file is
/// open if the PODIO:PRINT_TYPE_TABLE variable is set to a non-zero value
//------------------------------------------------------------------------------
void JEventSourcePODIO::PrintCollectionTypeTable() {

  // Read the zeroth entry. This assumes that m_reader has already been initialized with a valid filename
  auto frame_data = m_reader.readEntry("events", 0);
  auto frame      = std::make_unique<podio::Frame>(std::move(frame_data));

  std::map<std::string, std::string> collectionNames;
  std::size_t max_name_len = 0;
  std::size_t max_type_len = 0;

  // Record all (collection name, value type name) pairs
  // Record the maximum length of both strings so that we can print nicely aligned columns.
  for (const std::string& name : frame->getAvailableCollections()) {
    const podio::CollectionBase* coll = frame->get(name);
    const auto type                   = coll->getTypeName();
    max_name_len                      = std::max(max_name_len, name.length());
    max_type_len                      = std::max(max_type_len, type.length());
    collectionNames[name]             = std::string(type);
  }

  // Print table
  std::cout << std::endl;
  std::cout << "Available Collections" << std::endl;
  std::cout << std::endl;
  std::cout << "Collection Name"
            << std::string(max_name_len + 2 - std::string("Collection Name").length(), ' ')
            << "Data Type" << std::endl;
  std::cout << std::string(max_name_len, '-') << "  " << std::string(max_name_len, '-')
            << std::endl;
  for (auto& [name, type] : collectionNames) {
    std::cout << name + std::string(max_name_len + 2 - name.length(), ' ');
    std::cout << type << std::endl;
  }
  std::cout << std::endl;
}
