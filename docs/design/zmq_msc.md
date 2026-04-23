# EICrecon Managed PODIO Processing Sequence Diagram

```mermaid
sequenceDiagram
    participant Client as Python Client
    participant Listener as Listener Thread
    participant Processor as JEventProcessorManagedPODIO
    participant Source as JEventSourceManagedPODIO
    participant JANA as JANA Event Loop

    Note over Client, JANA: System Initialization
    Processor->>Processor: Init() - Create ZMQ socket, bind to /tmp/eicrecon_managed.sock
    Processor->>Listener: Start listener thread
    Listener->>Listener: ListenForMessages() - Poll ZMQ socket
    Source->>Source: Open() - Wait for file requests
    JANA->>JANA: Start event processing loop

    Note over Client, JANA: File Processing Request
    Client->>Listener: ZMQ REQ: {"input_file": "...", "output_file": "..."}

    Listener->>Listener: Receive ZMQ message
    Listener->>Processor: ProcessFileRequest(json)
    Processor->>Processor: Validate input file exists
    Processor->>Processor: Set m_pending_response = true
    Processor->>Processor: OpenOutputFile() - Create podio::Writer

    Processor->>Source: NotifySourceNewFile(input_file, output_file)
    Source->>Source: SetCurrentFile() - Reset state, open input file
    Source->>Source: ProcessCurrentFile() - Create podio::Reader
    Source->>Source: Set m_file_available = true, notify condition variable

    Note over Client, JANA: Event Processing Loop
    loop For each event in file
        JANA->>Source: Emit(event)
        Source->>Source: Check if file available, read next event
        Source->>Source: Insert collections into JEvent
        Source-->>JANA: Return Success

        JANA->>Processor: Process(event)
        Processor->>Processor: CheckFileCompletion() - Poll source
        Processor->>Source: IsFileProcessingComplete()
        Source-->>Processor: Return completion status

        alt File not complete
            Processor->>Processor: Write event to output file
            Processor->>Processor: Increment m_events_processed
        else File complete
            Processor->>Processor: CloseOutputFile()
            Processor->>Processor: Propagate non-event frames
            Processor->>Processor: Finish writer
            Processor->>Listener: SendResponse(completion_json)
            Listener->>Client: ZMQ REP: {"status": "completed", "events_processed": N}
            Processor->>Processor: Set m_file_processing_active = false
        end
    end

    Note over Source: End of file reached
    Source->>Source: Set m_file_processing_complete = true
    Source->>Source: Set m_file_available = false, reset reader
    Source-->>JANA: Return FailureTryAgain (wait for next file)

    Note over Client, JANA: Next File or Shutdown
    alt Another file request
        Client->>Listener: ZMQ REQ: {"input_file": "...", "output_file": "..."}
        Note over Client, JANA: Process repeats for new file
    else Shutdown
        Processor->>Listener: Stop listener thread
        Listener->>Listener: Clean up and exit
        Processor->>Processor: Clean up ZMQ socket
        Source->>Source: Close()
    end
```

## Key Points

1. **Initialization**: Processor creates ZMQ socket and starts listener thread. Source waits for file requests.

2. **File Request**: Client sends JSON request with input/output file paths via ZMQ REQ/REP pattern.

3. **Coordination**: Processor validates request, opens output file, then notifies source to open input file.

4. **Event Processing**: JANA event loop calls Source::Emit() to read events and Processor::Process() to write them. Processor polls source for completion status.

5. **Completion**: When source finishes reading all events, processor closes output file and sends completion response to client.

6. **Next File**: Source returns FailureTryAgain to keep JANA event loop alive, waiting for next file request.

## Communication Patterns

- **ZMQ REQ/REP**: Client ↔ Listener Thread (external communication)
- **Direct Method Calls**: Listener Thread → Processor, Processor → Source (internal coordination)
- **Polling**: Processor polls Source for completion status
- **Condition Variables**: Source uses CV to wait for new files
- **Threading**: Listener Thread runs independently, handling ZMQ communication asynchronously
