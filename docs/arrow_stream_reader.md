# Arrow IPC Stream Reader for EDM4hep

## Overview

This proof-of-concept implementation adds support for reading EDM4hep data from Apache Arrow IPC streams in EICrecon. This enables real-time streaming from simulation (npsim/ddsim) to reconstruction (eicrecon) without intermediate file I/O.

## Status: Proof-of-Concept

**Current Implementation:**
- ✅ Arrow dependency integrated into build system
- ✅ Arrow stream reader event source implemented (`JEventSourcePODIOArrowStream`)
- ✅ Code compiles and links successfully
- ✅ Integration framework in place
- ⚠️ Arrow-to-Frame conversion pending podio upgrade

**Pending Dependencies:**
1. DD4hep Arrow writer (PR: https://github.com/AIDASoft/DD4hep/pull/1658)
2. podio >= 1.8 with Arrow backend support (`ENABLE_ARROW`)

## Architecture

The Arrow stream reader is implemented as a JANA2 event source that:

1. Opens Apache Arrow IPC streams (files or named pipes)
2. Reads RecordBatches incrementally (one per event)
3. Converts RecordBatches to podio::Frame (when podio Arrow support is available)
4. Provides frames to the JANA2 framework for reconstruction

### Files

- `src/services/io/podio/JEventSourcePODIOArrowStream.h` - Header
- `src/services/io/podio/JEventSourcePODIOArrowStream.cc` - Implementation
- `src/tests/arrow_stream_test/` - CI test

## Usage (Future)

Once the dependencies are available, Arrow streams can be used as follows:

### File-based Input

```bash
# Create Arrow stream file with npsim
npsim --compactFile epic.xml \
      --outputFile simulation.arrow \
      -DD4hepOutput2EDM4hep.OutputBackend=arrow \
      --numberOfEvents 100

# Read with eicrecon
eicrecon simulation.arrow -Ppodio:output_file=reconstructed.root
```

Alternative using ddsim Python API:
```python
from DDSim.DD4hepSimulation import DD4hepSimulation
SIM = DD4hepSimulation()
SIM.compactFile = "epic.xml"
SIM.outputConfig.output = "simulation.arrow"
SIM.outputConfig.part.userParameters["OutputBackend"] = "arrow"
SIM.numberOfEvents = 100
SIM.run()
```

### Named Pipe Streaming

```bash
# Create named pipe
mkfifo simulation.arrow

# Start producer (background)
npsim --compactFile epic.xml \
      --outputFile simulation.arrow \
      -DD4hepOutput2EDM4hep.OutputBackend=arrow \
      --numberOfEvents 1000 &

# Start consumer
eicrecon simulation.arrow -Ppodio:output_file=reconstructed.root
```

### Real-time Streaming

Named pipes enable zero-latency streaming where reconstruction begins as soon as simulation produces the first event:

```bash
mkfifo /tmp/stream.arrow
npsim --outputFile /tmp/stream.arrow -DD4hepOutput2EDM4hep.OutputBackend=arrow &
eicrecon /tmp/stream.arrow &
```

## Configuration Parameters

The Arrow stream reader supports the same parameters as the standard PODIO reader:

- `podio:run_forever` - Recycle events continuously (default: false)

## Implementation Details

### Event Source Registration

The Arrow stream event source is registered with higher priority than the ROOT-based PODIO reader. File type is determined by extension:

- `.arrow`, `.arrowstream` → Arrow stream reader (priority 0.05)
- `.root` with `podio_metadata` → PODIO ROOT reader (priority 0.03)

### Data Format

The Arrow IPC stream format:
- One RecordBatch per event
- Each RecordBatch contains one row
- EDM4hep collections are stored as List<Struct> columns
- Collection metadata in Arrow field metadata
- Compatible with podio's `convertFrameToTable()` output

### Current Limitations (POC)

1. **No Frame Conversion**: The proof-of-concept logs Arrow schema but cannot convert to podio::Frame
2. **Single Event Processing**: Stops after reading first event to demonstrate capability
3. **No Error Recovery**: Basic error handling only
4. **No Performance Optimization**: Zero-copy not yet implemented

## Building

The Arrow stream reader is built automatically when EICrecon is configured:

```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
cmake --build build --target install
```

Requirements:
- Apache Arrow >= 10.0.0
- podio >= 1.3 (1.8+ with Arrow support for full functionality)

## Testing

A proof-of-concept test is included:

```bash
ctest --test-dir build -V -R arrow_stream_test
```

The test verifies:
- Code compiles successfully
- Arrow event source is registered
- Integration framework is in place

## Future Work

### Short Term
1. Wait for DD4hep PR #1658 to merge
2. Update to podio >= 1.8 with Arrow support
3. Enable full Arrow-to-Frame conversion
4. Test end-to-end streaming

### Long Term
1. Performance optimization (zero-copy where possible)
2. Improved error handling and recovery
3. Socket-based streaming support
4. Parallel event processing with buffering
5. Schema evolution handling
6. Comprehensive benchmarking suite

## References

- DD4hep Arrow writer PR: https://github.com/AIDASoft/DD4hep/pull/1658
- podio Arrow backend: https://github.com/AIDASoft/podio/pull/999
- podio Frame conversion: https://github.com/AIDASoft/podio/pull/980
- Arrow IPC format: https://arrow.apache.org/docs/format/Columnar.html#ipc-streaming-format
- JANA2 documentation: https://jeffersonlab.github.io/JANA2/

## Acknowledgments

This work was developed to enable real-time streaming from simulation to reconstruction in the ePIC experiment at the Electron-Ion Collider.
