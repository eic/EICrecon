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
- Conditional compilation for podio 1.7/1.8 compatibility
- Automatic backend detection via file extension and magic bytes
- Higher priority than ROOT-based PODIO reader

**Runtime Behavior:**
- **With podio 1.8+**: Full Arrow-to-Frame conversion, processes all events
- **With podio 1.7**: Proof-of-concept mode, demonstrates stream reading, stops after first event

**Dependencies:**
- Apache Arrow >= 10.0.0
- podio >= 1.3 (podio >= 1.8 with `ENABLE_ARROW` for full functionality)
- DD4hep with Arrow writer support (merged in DD4hep PR #1658)

## Architecture

The Arrow stream reader is implemented as a JANA2 event source that:

1. Opens Apache Arrow IPC streams (files or named pipes)
2. Reads RecordBatches incrementally (one per event)
3. Converts RecordBatches to podio::Frame using `podio::convertTableToFrame()` (podio 1.8+)
4. Inserts collections into JEvents using the same visitor pattern as ROOT-based input
5. Provides frames to the JANA2 framework for reconstruction

### Conditional Compilation

The reader uses `__has_include()` to detect podio Arrow support at compile time:

```cpp
#if __has_include(<podio/utilities/ArrowFrameConverter.h>)
  // Full conversion enabled with podio 1.8+
  auto frame = podio::convertTableToFrame(table, 0);
  // Insert collections into JEvent...
#else
  // Proof-of-concept mode with podio 1.7
  // Logs RecordBatch structure, stops after first event
#endif
```

This allows the code to build successfully with either podio version and automatically enable full functionality when podio 1.8+ is available.

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
- Tracks position via internal counter
- Detects FIFOs via `stat()` and `S_ISFIFO()`
- Regular files still use `ReadableFile` for performance

```cpp
// Detect FIFOs and use non-seeking stream
struct stat st;
if (stat(resource.c_str(), &st) == 0 && S_ISFIFO(st.st_mode)) {
  int fd = open(resource.c_str(), O_RDONLY);
  m_input_stream = std::make_shared<FdReadOnlyInputStream>(fd, true);
} else {
  // Regular files use ReadableFile
  auto result = arrow::io::ReadableFile::Open(resource);
  m_input_stream = *result;
}
```

### Current Limitations

**Podio Version Compatibility:**
- **With podio 1.7**: Proof-of-concept mode that demonstrates stream opening and RecordBatch reading, but stops after the first event (Frame conversion not available)
- **With podio 1.8+**: Full Arrow-to-Frame conversion enabled, processes all events and inserts collections into JEvents

**Implementation Status:**
1. `run_forever` not implemented (requires stream reopening logic)
2. Performance not yet optimized. Potential optimization opportunities:
   - **Zero-copy Arrow → podio**: Currently converts RecordBatch → Table → Frame with data copies. podio's `convertTableToFrame()` could be optimized to use Arrow's buffer pointers directly for primitive types.
   - **FIFO read buffering**: The `FdReadOnlyInputStream` currently uses a single 64KB buffer. Implementing double-buffering or ring buffers could reduce pipeline stalls when simulation and reconstruction rates are mismatched.
   - **Schema caching**: The Arrow schema is currently re-parsed for every RecordBatch. Caching the validated schema after the first event would eliminate redundant work.
   - **Collection pre-allocation**: Frame collection sizes are known from Arrow List lengths before conversion. Pre-allocating podio collections to exact sizes would reduce allocator overhead.
   - **Parallel conversion**: For large events with many collections, converting independent collections (e.g., different detector hits) in parallel using TBB or std::execution could improve throughput.
3. Basic error handling only

## Building

The Arrow stream reader is built automatically when EICrecon is configured:

```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
cmake --build build --target install
```

Requirements:
- Apache Arrow >= 10.0.0
- podio >= 1.3 (1.8+ with Arrow support for full functionality)

The build automatically detects podio Arrow support via `__has_include()`.

## Testing

The Arrow stream reader can be tested with either podio version:

**With podio 1.7 (proof-of-concept mode):**
```bash
ctest --test-dir build -V -R arrow_stream_test
```

Verifies that the code compiles, the Arrow event source is registered, Arrow streams can be opened, and RecordBatch structure is logged.

**With podio 1.8+ (full functionality):**
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

- DD4hep Arrow writer: https://github.com/AIDASoft/DD4hep/pull/1658
- podio Arrow backend: https://github.com/AIDASoft/podio/pull/999
- podio Frame conversion: https://github.com/AIDASoft/podio/pull/980
- Arrow IPC format specification: https://arrow.apache.org/docs/format/Columnar.html#ipc-streaming-format
- JANA2 documentation: https://jeffersonlab.github.io/JANA2/

## Acknowledgments

This work enables real-time streaming from simulation to reconstruction for the ePIC experiment at the Electron-Ion Collider.
