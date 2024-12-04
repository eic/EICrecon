```bash
    #run reconstruction
    eicrecon  -Pjana:nevents=${NEVE} -Pplugins=pid_angleres -Pdd4hep:xml_files=${XMLPATH}/${XMLFILE} -Phistsfile=${OUTDIR}/rootfiles/eicrecon_${EICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg.ana.root -Ppodio:output_file=${OUTDIR}/rootfiles/eicrecon_${EICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg.podio.root ${INDIR}/npsim_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg_1.edm4eic.root >> ${OUTDIR}/logs/eicrecon_${EPICVER}_${EPICVER}_${PART}_${MOM}GeV_${THMIN}deg_${THMAX}deg.out
```
