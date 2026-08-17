// Copyright 2024, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// Apache Arrow IPC Stream Reader for EDM4hep
// Compatible with DD4hep Geant4Output2EDM4hepArrowStream writer
//
// This event source reads Arrow IPC streams (files or named pipes) containing
// EDM4hep data written by DD4hep's Geant4Output2EDM4hepArrowStream action.
//
// NOTE: This is a proof-of-concept implementation. Full Arrow-to-Frame conversion
// requires podio with Arrow backend support (podio >= 1.8). This implementation
// provides the framework but currently logs an error for the actual conversion.

#include "JEventSourcePODIOArrowStream.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <arrow/api.h>
#include <arrow/array/array_base.h>
#include <arrow/io/file.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "services/io/podio/datamodel_glue.h"     // IWYU pragma: keep
#include "services/io/podio/datamodel_includes.h" // IWYU pragma: keep
#include "services/log/Log_service.h"

//------------------------------------------------------------------------------
// InsertingVisitor
//
/// This datamodel visitor will insert a PODIO collection into a JEvent.
/// This allows us to access the PODIO data through JEvent::Get and JEvent::GetCollection.
/// This makes it transparent to downstream factories whether the data was loaded from file, or calculated.
/// InsertingVisitor is called in Emit()
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
/// \param resource_name  Name of Arrow IPC stream file or named pipe to open
/// \param app            JApplication
//------------------------------------------------------------------------------
JEventSourcePODIOArrowStream::JEventSourcePODIOArrowStream(std::string resource_name,
                                                           JApplication* app)
    : JEventSource(resource_name, app) {
  SetTypeName(NAME_OF_THIS);                   // Provide JANA with class name
  SetCallbackStyle(CallbackStyle::ExpertMode); // Use new, exception-free Emit() callback

  // Get Logger
  m_log = GetApplication()->GetService<Log_service>()->logger("JEventSourcePODIOArrowStream");

  // Allow user to specify to recycle events forever
  GetApplication()->SetDefaultParameter("podio:run_forever", m_run_forever,
                                        "set to true to recycle through events continuously");
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
JEventSourcePODIOArrowStream::~JEventSourcePODIOArrowStream() {
  m_log->info("Closing Arrow Stream Event Source for {}", GetResourceName());
}

//------------------------------------------------------------------------------
// Open
//
/// Open the Arrow IPC stream and prepare for reading.
/// Supports both regular files and named pipes (FIFOs).
//------------------------------------------------------------------------------
void JEventSourcePODIOArrowStream::Open() {

  try {
    // Open the input stream (works for both files and named pipes)
    auto maybe_stream = arrow::io::ReadableFile::Open(GetResourceName());
    if (!maybe_stream.ok()) {
      throw JException(
          fmt::format("Failed to open Arrow stream: {}", maybe_stream.status().ToString()));
    }
    m_input_stream = *maybe_stream;

    // Open the Arrow IPC stream reader
    auto maybe_reader = arrow::ipc::RecordBatchStreamReader::Open(m_input_stream);
    if (!maybe_reader.ok()) {
      throw JException(fmt::format("Failed to create Arrow stream reader: {}",
                                   maybe_reader.status().ToString()));
    }
    m_arrow_reader = *maybe_reader;

    m_log->info("Opened Arrow IPC stream \"{}\"", GetResourceName());

    // Log the schema
    auto schema = m_arrow_reader->schema();
    m_log->debug("Arrow schema has {} fields:", schema->num_fields());
    for (int i = 0; i < schema->num_fields(); ++i) {
      auto field = schema->field(i);
      m_log->debug("  Field {}: {} ({})", i, field->name(), field->type()->ToString());
    }

  } catch (std::exception& e) {
    m_log->error(e.what());
    throw JException(fmt::format("Problem opening Arrow stream \"{}\"", GetResourceName()));
  }
}

//------------------------------------------------------------------------------
// Close
//
/// Cleanly close the resource when JANA is terminated via Ctrl-C or jana:nevents
//------------------------------------------------------------------------------
void JEventSourcePODIOArrowStream::Close() {
  if (m_arrow_reader) {
    auto status = m_arrow_reader->Close();
    if (!status.ok()) {
      m_log->warn("Error closing Arrow reader: {}", status.ToString());
    }
  }
  if (m_input_stream) {
    auto status = m_input_stream->Close();
    if (!status.ok()) {
      m_log->warn("Error closing input stream: {}", status.ToString());
    }
  }
}

//------------------------------------------------------------------------------
// Emit
//
/// Read next event from Arrow IPC stream and convert to podio::Frame.
/// Each event is one RecordBatch in the stream.
///
/// \param event
//------------------------------------------------------------------------------
JEventSourcePODIOArrowStream::Result JEventSourcePODIOArrowStream::Emit(JEvent& /* event */) {

  /// Calls to Emit are synchronized with each other, which means they can
  /// read and write state on the JEventSource without causing race conditions.

  try {
    // Read next RecordBatch from stream
    std::shared_ptr<arrow::RecordBatch> batch;
    auto status = m_arrow_reader->ReadNext(&batch);

    if (!status.ok()) {
      m_log->error("Error reading RecordBatch: {}", status.ToString());
      return Result::FailureTryAgain;
    }

    // End of stream
    if (batch == nullptr) {
      if (m_run_forever) {
        // TODO: For run_forever mode with streams, we would need to reopen the stream
        // For now, just return finished
        m_log->info("End of Arrow stream reached (run_forever not supported for streams)");
        return Result::FailureFinished;
      }
      return Result::FailureFinished;
    }

    // Convert RecordBatch to Arrow Table (needed for podio conversion)
    // Each RecordBatch is one event (one row in the table)
    auto table = arrow::Table::FromRecordBatches({batch});
    if (!table.ok()) {
      m_log->error("Failed to create Table from RecordBatch: {}", table.status().ToString());
      return Result::FailureTryAgain;
    }

    // NOTE: Arrow-to-Frame conversion requires podio with Arrow backend support
    // (podio >= 1.8 with ENABLE_ARROW). The convertTableToFrame function is not
    // available in podio 1.7 used in current eic-shell.
    //
    // For now, this proof-of-concept logs the Arrow schema to demonstrate that
    // the stream can be opened and read successfully.
    //
    // Once podio with Arrow support is available, uncomment this code:
    // #include <podio/utilities/ArrowFrameConverter.h>
    // auto frame_result = podio::convertTableToFrame(*table.ValueOrDie());
    // if (!frame_result.has_value()) {
    //   m_log->error("Failed to convert Arrow Table to podio Frame");
    //   return Result::FailureTryAgain;
    // }
    // auto frame = std::make_unique<podio::Frame>(std::move(frame_result.value()));

    m_log->info("Successfully read Arrow RecordBatch for event {}", m_events_read + 1);
    m_log->info("RecordBatch contains {} columns with {} rows:", batch->num_columns(),
                batch->num_rows());
    for (int i = 0; i < batch->num_columns(); ++i) {
      auto field = batch->schema()->field(i);
      m_log->info("  Column {}: {} - {} ({} elements)", i, field->name(), field->type()->ToString(),
                  batch->column(i)->length());
    }

    m_events_read += 1;

    // Proof-of-concept: Return finished after reading one event to demonstrate capability
    // In production with podio Arrow support, this would insert the frame into the JEvent
    // and return Result::Success to process multiple events
    m_log->warn("Arrow-to-Frame conversion requires podio >= 1.8 with Arrow support");
    m_log->warn("This proof-of-concept demonstrates successful Arrow stream reading");
    m_log->warn("Stopping after first event - full implementation pending podio upgrade");

    return Result::FailureFinished;

  } catch (std::exception& e) {
    m_log->error("Exception in Emit: {}", e.what());
    return Result::FailureTryAgain;
  }
}

//------------------------------------------------------------------------------
// GetDescription
//------------------------------------------------------------------------------
std::string JEventSourcePODIOArrowStream::GetDescription() { return "Arrow IPC Stream (EDM4hep)"; }

//------------------------------------------------------------------------------
// CheckOpenable
//
/// Return a value from 0-1 indicating probability that this source will be
/// able to read this Arrow stream. Currently, it checks for .arrow or .arrowstream
/// file extensions and returns a score slightly higher than PODIO ROOT files.
///
/// \param resource_name name of Arrow stream file to evaluate.
/// \return              value from 0-1 indicating confidence that this source can open the given file
//------------------------------------------------------------------------------
template <>
double
JEventSourceGeneratorT<JEventSourcePODIOArrowStream>::CheckOpenable(std::string resource_name) {

  // First check for Arrow-related file extensions (fast path)
  if (resource_name.find(".arrow") != std::string::npos ||
      resource_name.find(".arrowstream") != std::string::npos) {
    return 0.05; // Higher than PODIO ROOT (0.03)
  }

  // For other files (including named pipes), try to open and check for Arrow IPC magic bytes
  // Arrow IPC streams start with 0xFFFFFFFF followed by schema metadata
  auto result = arrow::io::ReadableFile::Open(resource_name);
  if (!result.ok()) {
    return 0.0; // Can't open file
  }

  auto input_file = result.ValueOrDie();

  // Read first 4 bytes to check for Arrow IPC magic number (0xFFFFFFFF)
  auto buffer_result = input_file->Read(4);
  if (!buffer_result.ok()) {
    return 0.0; // Can't read
  }

  auto buffer = buffer_result.ValueOrDie();
  if (buffer->size() < 4) {
    return 0.0; // Too small
  }

  // Check for Arrow IPC stream magic bytes: 0xFFFFFFFF (little-endian)
  const uint8_t* data = buffer->data();
  if (data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF) {
    return 0.05; // This is an Arrow IPC stream
  }

  return 0.0;
}
