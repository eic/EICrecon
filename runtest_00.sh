#!/bin/bash
export EPICVER=25.04.1
export XMLPATH=/eic/u/mposik/Detector1/EPIC/epic/install/share/epic
export XMLFILE=epic_craterlake_tracking_only.xml
export EICVER=v1.24.0
export NEVE=10000
export OUTDIR=/gpfs02/eic/mposik/ePIC/dirc_angleres/eicrecon
export INDIR=/gpfs02/eic/mposik/ePIC/dirc_angleres/npsim/rootfiles
export PART=pi-

#phimin:224,224,254,284,314
#phimax:312,282,282,312,342
#0-360, 12-40
#loop over theta ranges
THMINARRAY=(14 16 18 20 22 24 26 28 30 32 34 36 38)
PHIMIN=0.0
PHIMAX=360.0
for THMIN in "${THMINARRAY[@]}"
do
  THMIN_INT=${THMIN%.*}  # Remove the decimal part to get an integer
  THMAX=$((THMIN_INT + 2)) # Calculate THMAX as an integer
  echo "THMIN = ${THMIN}, THMAX = ${THMAX}" 

#loop over momentum values
  #MOMARRAY=(6.0 2.0 10.0)
  MOMARRAY=(6.0)
  for MOM in "${MOMARRAY[@]}"
  do
    echo "MOM = ${MOM}"
    #echo "EICrecon version: "
    eicrecon --version >> ${OUTDIR}/logs/eicrecon_${EPICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg_Phi_${PHIMIN}deg_${PHIMAX}deg_my_tracking_only.out

    #run reconstruction
    eicrecon -Pjana:nevents=${NEVE} -Pdd4hep:xml_files=${XMLPATH}/${XMLFILE} -Pplugins=pid_angleres -PMPGD:SiFactoryPattern=0x3 -Phistsfile=${OUTDIR}/rootfiles/eicrecon_${EICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg_Phi_${PHIMIN}deg_${PHIMAX}deg_my_tracking_only_map.ana.root -Ppodio:output_file=${OUTDIR}/rootfiles/eicrecon_${EICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg_Phi_${PHIMIN}deg_${PHIMAX}deg_my_tracking_only_map.podio.root ${INDIR}/npsim_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg_Phi_${PHIMIN}deg_${PHIMAX}deg_tracking_only.edm4eic.root >> ${OUTDIR}/logs/eicrecon_${EPICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg_Phi_${PHIMIN}deg_${PHIMAX}deg_my_tracking_only.out
  done
done

