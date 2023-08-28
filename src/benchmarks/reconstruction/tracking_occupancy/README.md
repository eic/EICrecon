This is guinea pig plugin for active development phase

```bash

# uniform spread inside an angle:

ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile tracking_test_gun.root


ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-09-04_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_2k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=10000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_10k.edm4hep.root

# 5 x e- per event
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.multiplicity 3 --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-09-10_pgun_3xe-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root
podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root

--gun.momentumMax
--gun.momentumMin


-Pplugins=acts,tracking,BTRK,ECTRK,BVTX,MPGD,tracking_occupancy
-Pnthreads=1
-Pjana:debug_plugin_loading=1
-Pjana:nevents=1
-Pjana:debug_mode=1
-Pjana:timeout=0
-PTracking:CentralTrackerSourceLinker:LogLevel=info
-PCKFTracking:Trajectories:LogLevel=trace
-Pdd4hep:xml_files=/home/romanov/eic/soft/detector/main/compiled/epic/share/epic/epic_tracking_only.xml
-Phistsfile=/home/romanov/work/data/eicrecon_test/tracking_test_gun.ana.root
/home/romanov/work/data/eicrecon_test/2022-09-10_pgun_3xe-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root

```

```bash
eicrecon
-Pplugins=acts,BTRK_test,BTRK
-Pnthreads=1
-Pjana:nevents=10
-Pjana:debug_plugin_loading=1
-PSiliconTrackerDigi_BarrelTrackerRawHit:LogLevel=trace
-PTrackerHitReconstruction:BarrelTrackerHit:LogLevel=trace
-PTrackerSourceLinker:CentralTrackerSourceLinker:LogLevel=trace
-Pdd4hep:print_level=5
-Phistsfile=/home/romanov/work/data/eicrecon_test/tracking_test_gun.ana.root
/home/romanov/work/data/eicrecon_test/2022-09-04_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_100ev.edm4hep.root
```
