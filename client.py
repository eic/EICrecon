#!/usr/bin/env python3
"""
Simple client for EICrecon managed PODIO processor.
Submits a file processing request and listens for all responses.
"""

import zmq
import json
import sys
import argparse
import time
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description='Submit file to EICrecon managed PODIO processor')
    parser.add_argument('input_file', help='Input file path')
    parser.add_argument('output_file', help='Output file path')
    parser.add_argument('--socket', default='/tmp/eicrecon_managed.sock',
                       help='Socket path (default: /tmp/eicrecon_managed.sock)')
    parser.add_argument('--timeout', type=int, default=300,
                       help='Timeout in seconds (default: 300)')

    args = parser.parse_args()

    # Validate input file exists
    if not Path(args.input_file).exists():
        print(f"Error: Input file '{args.input_file}' does not exist")
        sys.exit(1)

    # Create ZeroMQ context and socket
    context = zmq.Context()
    socket = context.socket(zmq.REQ)

    try:
        # Connect to the managed processor
        socket_address = f"ipc://{args.socket}"
        print(f"Connecting to {socket_address}...")
        socket.connect(socket_address)

        # Set socket timeout
        socket.setsockopt(zmq.RCVTIMEO, args.timeout * 1000)  # Convert to milliseconds
        socket.setsockopt(zmq.SNDTIMEO, 5000)  # 5 second send timeout

        # Prepare request
        request = {
            "input_file": str(Path(args.input_file).absolute()),
            "output_file": str(Path(args.output_file).absolute())
        }

        print(f"Submitting request:")
        print(f"  Input:  {request['input_file']}")
        print(f"  Output: {request['output_file']}")

        # Send request
        socket.send_string(json.dumps(request))

        # Wait for single response
        start_time = time.time()
        print("Waiting for processing to complete...")

        try:
            # Receive response
            response_str = socket.recv_string()
            response = json.loads(response_str)

            elapsed = time.time() - start_time
            print(f"\n[{elapsed:.1f}s] Response received:")
            print(f"  Status: {response.get('status', 'unknown')}")

            if 'message' in response:
                print(f"  Message: {response['message']}")

            if 'events_processed' in response:
                print(f"  Events processed: {response['events_processed']}")

            # Check final status
            status = response.get('status', '')
            if status == 'completed':
                print(f"\n✓ Processing completed successfully!")
                print(f"  Total time: {elapsed:.1f}s")
                print(f"  Output file: {response.get('output_file', args.output_file)}")
            elif status == 'error':
                print(f"\n✗ Processing failed!")
                print(f"  Error: {response.get('message', 'Unknown error')}")
                sys.exit(1)
            else:
                print(f"\n? Unexpected status: {status}")
                sys.exit(1)

        except zmq.Again:
            print(f"\nTimeout after {args.timeout} seconds")
            sys.exit(1)
        except KeyboardInterrupt:
            print(f"\nInterrupted by user")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"\nError parsing response: {e}")
            print(f"Raw response: {response_str}")
            sys.exit(1)

    except zmq.ZMQError as e:
        print(f"ZMQ Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)
    finally:
        socket.close()
        context.term()


if __name__ == "__main__":
    main()
