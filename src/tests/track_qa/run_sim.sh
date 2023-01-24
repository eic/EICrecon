#!/bin/bash

source /opt/detector/setup.sh

#ddsim --steeringFile mysteer.py --compactFile $DETECTOR_PATH/epic.xml --outputFile output.edm4hep.root

source /gpfs02/eic/baraks/epic/EICRecon/EICrecon/bin/eicrecon-this.sh

eicrecon \
-Pplugins=dump_flags,track_qa \
-Ppodio:output_file=eicrecon_out.root \
-Ptrack_qa:LogLevel=trace \
-Pjana:nevents=10 \
-Pdd4hep:xml_files=epic.xml \
output.edm4hep.root | tee eicrecon_out.dat
