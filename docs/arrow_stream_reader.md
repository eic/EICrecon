# Arrow IPC Stream Reader for EDM4hep

## ⚠️ Format Verification

The Arrow stream reader expects Apache Arrow IPC format. Verify your input file is correct:

```bash
xxd -l 4 stream.arrow  # Should show: ffff ffff (Arrow magic)
                        # NOT: 726f 6f74 ("root" magic)
```

**Common Error:** If you see `Expected to read N metadata bytes, but only read 0`, your file is likely ROOT format, not Arrow IPC format.

## Overview

The Arrow IPC stream reader (`JEventSourcePODIOArrowStream`) enables real-time streaming from simulation (npsim/ddsim) to reconstruction (eicrecon) without intermediate file I/O. It reads EDM4hep data from Apache Arrow IPC streams and provides events to the JANA2 reconstruction framework.

**Key Features:**
- File and named pipe (FIFO) input support
- Non-seeking stream handling for real-time data
- Automatic backend detection via file extension and magic bytes
- Higher priority than ROOT-based PODIO reader

**Dependencies:**
- Apache Arrow >= 10.0.0
- podio built with Arrow support (exports `podio::podioArrow` and provides `podio/utilities/ArrowFrameConverter.h`, e.g. podio 1.8)
- DD4hep with Arrow writer support to produce streams from npsim/ddsim (proposed in DD4hep PR #1658, not yet merged)

## Architecture

The Arrow stream reader is implemented as a JANA2 event source that:

1. Opens Apache Arrow IPC streams (files or named pipes)
2. Reads RecordBatches incrementally (one per event)
3. Converts RecordBatches to podio::Frame using `podio::convertTableToFrame()`
4. Sets the JEvent event and run numbers from the `EventHeader` collection, as the ROOT-based reader does
5. Inserts collections into JEvents using the same visitor pattern as ROOT-based input
6. Provides frames to the JANA2 framework for reconstruction

### Files

- `src/services/io/podio/JEventSourcePODIOArrowStream.h` - Event source header
- `src/services/io/podio/JEventSourcePODIOArrowStream.cc` - Event source implementation
- `src/services/io/podio/podio.cc` - Registration (registered before ROOT reader)
- `src/tests/arrow_stream_test/` - CI test infrastructure

## Usage

### File-based Input

```bash
# Create Arrow stream file with DD4hep simulation
# (DD4hep configuration varies - consult DD4hep documentation)
npsim --compactFile epic.xml \
      --outputFile simulation.arrow \
      --numberOfEvents 100

# Read with eicrecon
eicrecon simulation.arrow -Ppodio:output_file=reconstructed.root
```

### Named Pipe Streaming

```bash
# Create named pipe
mkfifo simulation.arrow

# Start producer (background)
# (DD4hep configuration varies - consult DD4hep documentation)
npsim --compactFile epic.xml \
      --outputFile simulation.arrow \
      --numberOfEvents 1000 &

# Start consumer
eicrecon simulation.arrow -Ppodio:output_file=reconstructed.root -Pnthreads=1
```

**Note:** Use `-Pnthreads=1` with named pipes to ensure sequential event processing from the single-stream FIFO.

### Real-time Streaming

Named pipes enable zero-latency streaming where reconstruction begins as soon as simulation produces the first event:

```bash
mkfifo /tmp/stream.arrow
# (DD4hep configuration varies - consult DD4hep documentation)
npsim --outputFile /tmp/stream.arrow --numberOfEvents 100 &
eicrecon /tmp/stream.arrow -Pnthreads=1 &
wait
```

## Configuration Parameters

The Arrow stream reader supports the same parameters as the standard PODIO reader. The `podio:run_forever` parameter is not yet implemented for Arrow streams.

## Implementation Details

### Event Source Registration

The Arrow stream event source is registered with higher priority than the ROOT-based PODIO reader:

- Arrow stream reader: priority 0.05 (file extensions) / 0.04 (FIFOs)
- ROOT PODIO reader: priority 0.03

File type determination:
1. **Fast path**: Check for `.arrow` or `.arrowstream` extension → confidence 0.05
2. **Magic bytes**: For other files, read first 4 bytes and check for Arrow IPC magic (0xFFFFFFFF)
3. **FIFO detection**: Use `stat()` to detect named pipes → confidence 0.04 (can't read without blocking)

### Data Format

The Arrow IPC stream format:
- **Magic bytes**: `0xFFFFFFFF` (4 bytes at start)
- **One RecordBatch per event**: Each batch is a 1-row table
- **Collections as columns**: EDM4hep collections stored as `List<Struct>` columns
- **Collection metadata**: Type information in Arrow field metadata (`value_type` → `"MCParticle"`, etc.)
- **Frame parameters**: Stored in `frame_parameters` column
- **Compatible**: Uses podio's `convertFrameToTable()` / `convertTableToFrame()` API

### Named Pipe (FIFO) Support

**Challenge**: Arrow's `ReadableFile` calls `lseek()` on all file descriptors, but FIFOs don't support seeking.

**Solution**: Custom `FdReadOnlyInputStream` class (mirroring DD4hep writer's approach):
- Wraps POSIX file descriptor using only `::read()` (never `lseek()`)
- Loops over partial reads and retries reads interrupted by signals (`EINTR`)
- Tracks position via internal counter
- Detects FIFOs via `stat()` and `S_ISFIFO()`
- Regular files still use `ReadableFile` for performance

```cpp
// Detect FIFOs and use non-seeking stream
struct stat st;
if (stat(resource.c_str(), &st) == 0 && S_ISFIFO(st.st_mode)) {
  int fd = open(resource.c_str(), O_RDONLY);
  m_input_stream = std::make_shared<FdReadOnlyInputStream>(fd);
} else {
  // Regular files use ReadableFile
  auto result = arrow::io::ReadableFile::Open(resource);
  m_input_stream = *result;
}
```

### Error Handling

Errors reading a RecordBatch, building the Arrow Table, or converting it to a podio Frame are logged and end the stream (`FailureFinished`). Processing stops after the events already read; these errors do not cause a non-zero exit code.

If an event does not contain exactly one `EventHeader` entry, a warning is logged once and event and run numbers are no longer taken from the header.

### Current Limitations

1. `run_forever` not implemented (requires stream reopening logic)
2. Performance not yet optimized. Potential optimization opportunities:
   - **Zero-copy Arrow → podio**: Currently converts RecordBatch → Table → Frame with data copies. podio's `convertTableToFrame()` could be optimized to use Arrow's buffer pointers directly for primitive types.
   - **FIFO read buffering**: The `FdReadOnlyInputStream` has no internal buffer and calls `::read()` directly into the buffers Arrow requests. Adding read-ahead buffering could reduce pipeline stalls when simulation and reconstruction rates are mismatched.
   - **Collection pre-allocation**: Frame collection sizes are known from Arrow List lengths before conversion. Pre-allocating podio collections to exact sizes would reduce allocator overhead.
   - **Parallel conversion**: For large events with many collections, converting independent collections (e.g., different detector hits) in parallel using TBB or std::execution could improve throughput.

The Arrow schema is read once when the stream is opened; it is not re-parsed per RecordBatch.

## Building

The Arrow stream reader is built automatically when EICrecon is configured:

```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
cmake --build build --target install
```

Requirements:
- Apache Arrow >= 10.0.0
- podio built with Arrow support (`podio::podioArrow` target and `podio/utilities/ArrowFrameConverter.h`)

The top-level `CMakeLists.txt` sets `EICRECON_HAS_ARROW` only when both requirements are met, and reports either `Arrow stream reader enabled` or `Arrow stream reader disabled` at configure time. When disabled, the Arrow stream reader sources are excluded from the podio plugin and `arrow_stream_test` is skipped; the rest of EICrecon builds normally.

## Testing

When the Arrow stream reader is enabled, `arrow_stream_test` checks the installed build:

```bash
ctest --test-dir build -V -R arrow_stream_test
```

It fails unless the installed `eicrecon` executable and `podio.so` plugin exist and the plugin contains the `JEventSourcePODIOArrowStream` event source. It does not stream events.

To test end-to-end streaming (requires DD4hep with the Arrow writer):
```bash
# Create named pipe
mkfifo test_stream.arrow

# Producer (background)
# (DD4hep configuration varies - consult DD4hep documentation)
npsim --compactFile epic.xml \
      --outputFile test_stream.arrow \
      --numberOfEvents 10 &

# Consumer
eicrecon test_stream.arrow -Ppodio:output_file=test_output.root -Pnthreads=1

# Verify output
ls -lh test_output.root
```

## References

- DD4hep Arrow writer (open): https://github.com/AIDASoft/DD4hep/pull/1658
- podio Frame → Table conversion: https://github.com/AIDASoft/podio/pull/980
- podio Table → Frame conversion: https://github.com/AIDASoft/podio/pull/987
- podio Arrow CMake target export: https://github.com/AIDASoft/podio/pull/994
- Arrow IPC format specification: https://arrow.apache.org/docs/format/Columnar.html#ipc-streaming-format
- JANA2 documentation: https://jeffersonlab.github.io/JANA2/

## Acknowledgments

This work enables real-time streaming from simulation to reconstruction for the ePIC experiment at the Electron-Ion Collider.
