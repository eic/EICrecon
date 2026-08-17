#!/bin/bash
set -e

# Test Arrow IPC streaming from ddsim to eicrecon

echo "=== Arrow Stream Test ==="
echo

# Setup environment
if [ -z "$DETECTOR_PATH" ]; then
    echo "ERROR: DETECTOR_PATH not set. Run: source /opt/detector/epic-main/bin/thisepic.sh"
    exit 1
fi

DETECTOR_CONFIG="${DETECTOR}_craterlake"

# Clean up any previous test files
rm -f test_stream.arrow test_stream.fifo

echo "Step 1: Generate Arrow stream file with ddsim"
echo "Command: ddsim --compactFile=$DETECTOR_PATH/$DETECTOR_CONFIG.xml \\"
echo "         --outputFile=test_stream.arrow \\"
echo "         --output.part.userParameters OutputBackend=arrow \\"
echo "         -N2 -G --gun.particle e- --gun.distribution uniform --gun.energy 1*GeV"
echo

ddsim --compactFile=$DETECTOR_PATH/$DETECTOR_CONFIG.xml \
      --outputFile=test_stream.arrow \
      --output.part.userParameters OutputBackend=arrow \
      -N2 \
      -G \
      --gun.particle e- \
      --gun.distribution uniform \
      --gun.energy 1*GeV

if [ ! -f test_stream.arrow ]; then
    echo "ERROR: ddsim did not create test_stream.arrow"
    exit 1
fi

echo
echo "Step 2: Verify Arrow file was created"
ls -lh test_stream.arrow
echo

# Check if it's a valid Arrow file (magic bytes: 0xFFFFFFFF)
echo "Step 3: Check Arrow magic bytes (should be FF FF FF FF)"
xxd -l 8 test_stream.arrow

echo
echo "Step 4: Read stream with eicrecon"
echo "Command: eicrecon test_stream.arrow -Pnthreads=1 -Pjana:nevents=2"
echo

eicrecon test_stream.arrow -Pnthreads=1 -Pjana:nevents=2

echo
echo "=== Test Complete ==="
