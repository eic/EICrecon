#!/bin/bash

source /opt/detector/setup.sh

#ddsim --steeringFile mysteer_etarange.py --compactFile $DETECTOR_PATH/epic.xml --outputFile output.edm4hep.root

npsim --compactFile $DETECTOR_PATH/epic.xml --enableGun --gun.distribution 'eta' \
--gun.thetaMax 3.106 --gun.thetaMin 0.036 --gun.momentumMin "0.5*GeV" --gun.momentumMax "20*GeV" \
--numberOfEvents 10000 --outputFile output.edm4hep.root

source ../../../bin/eicrecon-this.sh

eicrecon \
-Pplugins=dump_flags,track_qa \
-Ppodio:output_file=eicrecon_out.root \
-Ppodio:output_include_collections=MCParticles,CentralTrackSeedingResults \
-Ptrack_qa:LogLevel=trace \
-Pjana:nevents=10000 \
-Pdd4hep:xml_files=epic.xml \
output.edm4hep.root | tee eicrecon_out.dat
