```bash
eicrecon
-Pplugins=dump_flags,track_propagation_test
-Ppodio:output_file=/home/romanov/work/data/eicrecon_test/processed.ana.root
-Ptrack_propagation_test:LogLevel=trace
-Pjana:nevents=10000
-Pdd4hep:xml_files=epic_arches.xml
-Phistsfile=/home/romanov/work/data/eicrecon_test/histogram.ana.root
/home/romanov/eic/soft/eicrecon/main/src/examples/test_data_generator/2022-11-15_pgun_pi-_epic_arches_e0.01-30GeV_alldir_4prt_1000evt.edm4hep.root
```

