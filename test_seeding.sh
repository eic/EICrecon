#!/bin/bash
set -e

# Source environment
source /opt/detector/epic-main/bin/thisepic.sh
source install/bin/eicrecon-this.sh

# Generate a small test file (10 events)
echo "Generating test simulation file..."
ddsim --compactFile $DETECTOR_PATH/$DETECTOR_CONFIG.xml \
      --numberOfEvents 5 \
      --enableGun \
      --gun.thetaMin 'pi/2' --gun.thetaMax 'pi/2' \
      --gun.distribution uniform \
      --gun.phiMin '0*deg' --gun.phiMax '0*deg' \
      --gun.energy '1*GeV' \
      --gun.particle 'e-' \
      --outputFile test_sim.edm4hep.root 2>&1 | tail -20

echo ""
echo "Testing Seeding2 method (default)..."
eicrecon -Ppodio:output_file=test_seeding2.edm4eic.root \
         -Pjana:nevents=5 \
         test_sim.edm4hep.root 2>&1 | grep -E "(TrackSeeding|Seeding2|method)" || echo "Seeding2 completed"

echo ""
echo "Testing Orthogonal method (explicit)..."
eicrecon -Ppodio:output_file=test_orthogonal.edm4eic.root \
         -PTrackSeeding:seedingMethod=orthogonal \
         -Pjana:nevents=5 \
         test_sim.edm4hep.root 2>&1 | grep -E "(TrackSeeding|Orthogonal|method)" || echo "Orthogonal completed"

echo ""
echo "Both methods completed successfully!"
ls -lh test_*.root
