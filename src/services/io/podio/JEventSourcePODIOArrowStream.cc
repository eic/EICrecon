// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, EICrecon contributors
//
// Apache Arrow IPC Stream Reader for EDM4hep
// Compatible with DD4hep Geant4Output2EDM4hep writer with Arrow backend
//
// This event source reads Arrow IPC streams (files or named pipes) containing
// EDM4hep data written by DD4hep's Geant4Output2EDM4hep with Arrow backend.

#include "JEventSourcePODIOArrowStream.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Utils/JTypeInfo.h>
#include <arrow/api.h>
#include <arrow/array/array_base.h>
#include <arrow/io/file.h>
#include <arrow/io/interfaces.h>
#include <fmt/format.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <vector>

// Check if podio Arrow support is available
#if __has_include(<podio/utilities/ArrowFrameConverter.h>)
#include <podio/utilities/ArrowFrameConverter.h>
#define PODIO_ARROW_SUPPORT 1
#else
#define PODIO_ARROW_SUPPORT 0
#endif

#include "services/io/podio/datamodel_glue.h"     // IWYU pragma: keep
#include "services/io/podio/datamodel_includes.h" // IWYU pragma: keep
#include "services/log/Log_service.h"

//------------------------------------------------------------------------------
// FdReadOnlyInputStream
//
/// A minimal Arrow InputStream that wraps a POSIX file descriptor using
/// only read() (never lseek). This is required for named pipes (FIFOs)
/// which don't support seeking operations.
///
/// Based on the DD4hep FdWriteOnlyOutputStream implementation for the writer.
//------------------------------------------------------------------------------
class FdReadOnlyInputStream : public arrow::io::InputStream {
private:
  int m_fd;
  int64_t m_position;
  bool m_closed;

public:
  explicit FdReadOnlyInputStream(int fd) : m_fd(fd), m_position(0), m_closed(false) {}

  ~FdReadOnlyInputStream() override {
    if (!m_closed && m_fd >= 0) {
      ::close(m_fd);
    }
  }

  arrow::Status Close() override {
    if (!m_closed && m_fd >= 0) {
      if (::close(m_fd) != 0) {
        return arrow::Status::IOError("close() failed");
      }
      m_closed = true;
    }
    return arrow::Status::OK();
  }

  arrow::Result<int64_t> Tell() const override { return m_position; }

  bool closed() const override { return m_closed; }

  arrow::Result<int64_t> Read(int64_t nbytes, void* out) override {
    if (m_closed) {
      return arrow::Status::Invalid("Stream is closed");
    }

    // Loop until we read all requested bytes or reach EOF
    // This is critical for FIFOs where read() may return partial data
    int64_t total_read = 0;
    char* buffer       = static_cast<char*>(out);

    while (total_read < nbytes) {
      ssize_t n = ::read(m_fd, buffer + total_read, nbytes - total_read);
      if (n < 0) {
        return arrow::Status::IOError("read() failed");
      }
      if (n == 0) {
        // EOF reached
        break;
      }
      total_read += n;
    }

    m_position += total_read;
    return total_read;
  }

  arrow::Result<std::shared_ptr<arrow::Buffer>> Read(int64_t nbytes) override {
    if (m_closed) {
      return arrow::Status::Invalid("Stream is closed");
    }
    auto buffer_result = arrow::AllocateBuffer(nbytes);
    if (!buffer_result.ok()) {
      return buffer_result.status();
    }
    auto buffer      = std::move(buffer_result).ValueOrDie();
    auto read_result = Read(nbytes, buffer->mutable_data());
    if (!read_result.ok()) {
      return read_result.status();
    }
    int64_t bytes_read = read_result.ValueOrDie();
    if (bytes_read < nbytes) {
      // Create a new buffer with the actual size read
      auto smaller_result = arrow::AllocateBuffer(bytes_read);
      if (!smaller_result.ok()) {
        return smaller_result.status();
      }
      auto smaller_buffer = std::move(smaller_result).ValueOrDie();
      memcpy(smaller_buffer->mutable_data(), buffer->data(), bytes_read);
      return smaller_buffer;
    }
    return buffer;
  }
};

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
    std::string resource_name = GetResourceName();

    // Check if this is a FIFO (named pipe) - they don't support lseek
    struct stat st;
    bool is_fifo = false;
    if (stat(resource_name.c_str(), &st) == 0) {
      is_fifo = S_ISFIFO(st.st_mode);
    }

    if (is_fifo) {
      // For FIFOs, use our custom non-seeking input stream
      m_log->debug("Opening FIFO \"{}\" with non-seeking stream", resource_name);
      int fd = ::open(resource_name.c_str(), O_RDONLY);
      if (fd < 0) {
        throw JException(fmt::format("Failed to open FIFO: {}", strerror(errno)));
      }
      m_input_stream = std::make_shared<FdReadOnlyInputStream>(fd);
    } else {
      // For regular files, use Arrow's standard file reader
      m_log->debug("Opening file \"{}\" with Arrow ReadableFile", resource_name);
      auto maybe_stream = arrow::io::ReadableFile::Open(resource_name);
      if (!maybe_stream.ok()) {
        throw JException(
            fmt::format("Failed to open Arrow stream: {}", maybe_stream.status().ToString()));
      }
      m_input_stream = *maybe_stream;
    }

    // Open the Arrow IPC stream reader
    auto maybe_reader = arrow::ipc::RecordBatchStreamReader::Open(m_input_stream);
    if (!maybe_reader.ok()) {
      throw JException(fmt::format("Failed to create Arrow stream reader: {}",
                                   maybe_reader.status().ToString()));
    }
    m_arrow_reader = *maybe_reader;

    m_log->info("Opened Arrow IPC stream \"{}\" ({})", resource_name, is_fifo ? "FIFO" : "file");

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
JEventSourcePODIOArrowStream::Result JEventSourcePODIOArrowStream::Emit(JEvent& event) {

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

#if PODIO_ARROW_SUPPORT
    // Convert Arrow Table to podio Frame using podio 1.8+ API
    m_log->debug("Converting Arrow Table to podio Frame for event {}", m_events_read + 1);
    auto frame = podio::convertTableToFrame(table.ValueOrDie(), 0);

    m_log->debug("Successfully converted Arrow Table to Frame with {} collections",
                 frame.getAvailableCollections().size());

    // Create a unique_ptr to the frame
    auto frame_ptr = std::make_unique<podio::Frame>(std::move(frame));

    // Insert contents of frame into JFactories using the same pattern as JEventSourcePODIO
    VisitPodioCollection<InsertingVisitor> visit;
    for (const std::string& coll_name : frame_ptr->getAvailableCollections()) {
      const podio::CollectionBase* collection = frame_ptr->get(coll_name);
      InsertingVisitor visitor(event, coll_name);
      visit(visitor, *collection);
    }

    // Transfer ownership from unique_ptr to JFactoryT<podio::Frame>
    event.Insert(frame_ptr.release());

    m_events_read += 1;

    return Result::Success;
#else
    // Fallback for podio < 1.8 without Arrow support
    m_log->info("Successfully read Arrow RecordBatch for event {}", m_events_read + 1);
    m_log->info("RecordBatch contains {} columns with {} rows:", batch->num_columns(),
                batch->num_rows());
    for (int i = 0; i < batch->num_columns(); ++i) {
      auto field = batch->schema()->field(i);
      m_log->info("  Column {}: {} - {} ({} elements)", i, field->name(), field->type()->ToString(),
                  batch->column(i)->length());
    }

    m_events_read += 1;

    // Log once per run that full support requires podio 1.8+
    static bool warned = false;
    if (!warned) {
      m_log->warn("Arrow-to-Frame conversion requires podio >= 1.8 with Arrow support");
      m_log->warn("Currently running in proof-of-concept mode: stream reading works,");
      m_log->warn("but Frame conversion is disabled. Upgrade podio to enable full functionality.");
      warned = true;
    }

    m_log->warn("Stopping after event {} - full implementation pending podio 1.8+", m_events_read);
    return Result::FailureFinished;
#endif

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

  // Check if this is a FIFO (named pipe)
  struct stat st;
  if (stat(resource_name.c_str(), &st) == 0 && S_ISFIFO(st.st_mode)) {
    // For FIFOs, we can't read magic bytes without blocking (no writer yet)
    // Return moderate confidence - the user explicitly provided this path
    return 0.04; // Slightly lower than extension match to prefer .arrow files
  }

  // For regular files, try to open and check for Arrow IPC magic bytes
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
