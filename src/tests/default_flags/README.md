This is guinea pig plugin for active development phase

```bash

# uniform spread inside an angle:

ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile tracking_test_gun.root


ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_2k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=10000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_10k.edm4hep.root

--gun.momentumMax
--gun.momentumMin

```

```bash
eicrecon
-Pplugins=BTRK,data_flow_test
-Pnthreads=1
-Pjana:debug_plugin_loading=1
-Pnthreads=1
/home/romanov/work/data/eicrecon_test/tracking_test_gun.root
-Phistsfile=/home/romanov/work/data/eicrecon_test/tracking_test_gun.ana.root
```