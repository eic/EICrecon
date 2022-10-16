This is guinea pig plugin for active development phase

```bash

-Pplugins=dump_flags
-Pdump_flags:python=/home/romanov/eic/soft/eicrecon/main/src/tools/default_values_table/all_flags_dump.py
-Pjana:debug_plugin_loading=1
-Ppodio:output_file=/home/romanov/work/data/eicrecon_test/eicrecon.ana.edm4hep.root
-Ptracking_test:LogLevel=trace
-Pjana:nevents=10
-Pacts:MaterialMap=/home/romanov/work/data/eicrecon_test/calibrations/materials-map.cbor
-Ptracking:ChargedParticlesWithAssociations:LogLevel=debug
-Pdd4hep:xml_files=epic.xml
-Phistsfile=/home/romanov/work/data/eicrecon_test/histogram.ana.root
/home/romanov/work/data/eicrecon_test/2022-10-06_pgun_TOF_1-pi+_en1-20GeV_alldir_2000evt.edm4hep.root
```
