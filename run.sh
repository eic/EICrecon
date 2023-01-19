#!/bin/bash

ENERGIES="10"
PARTICLES="photon"
DIR_NAME="campaign_noCladding"
DIR_PATH=/home/anl.gov/zurek/eic-benchmarks-epic/results/EICrecon/sim_output_${DIR_NAME}
mkdir ${DIR_PATH}

for p in $PARTICLES
do
    export E_file="sim_output_${DIR_NAME}/emcal_barrel_energy_scan_points_${p}.txt"
    for e in $ENERGIES
    do
	FILE_NAME=rec_emcal_barrel_${p}_${e}_DistFix.edm4eic.root
	LOG_NAME=${p}_${e}_flags_DistFix.json

	eicrecon -Ppodio:output_file=${FILE_NAME} /home/anl.gov/zurek/eic-benchmarks-epic/reconstruction_benchmarks/sim_output_${DIR_NAME}/sim_emcal_barrel_${p}_${e}.edm4hep.root -Pplugins=dump_flags,janadot -Pdump_flags:json=${LOG_NAME}
	mv ${FILE_NAME} ${DIR_PATH}/.
	mv ${LOG_NAME} ${DIR_PATH}/.
    done
done
