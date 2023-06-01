This is guinea pig plugin for active development phase

```bash

# uniform spread inside an angle:

ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile tracking_test_gun.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=200 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-09-04_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_200ev.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-09-04_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_2k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=10000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_10k.edm4hep.root

# 5 x e- per event
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.multiplicity 3 --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-09-10_pgun_3xe-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root
podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root

--gun.momentumMax
--gun.momentumMin

# Reconstruct Charge Current DIS
# Detectors live in /opt/detectors
# one can select particular configuration as
# source /opt/detector/athena-deathvalley-1.5T/setup.sh
#
# or one can set the latest detector
source /opt/detector/setup.sh

# 100 events
ddsim --compactFile=$DETECTOR_PATH/epic.xml --runType=run -N=100 --outputFile=2022-09-26_ccdis10x100_100ev.edm4hep.root --inputFiles pythia8CCDIS_10x100_minQ2=100_beamEffects_xAngle=-0.025_hiDiv_1.hepmc

# 200 events
ddsim --compactFile=$DETECTOR_PATH/epic.xml --runType=run -N=200 --outputFile=2022-09-26_ccdis10x100_200ev.edm4hep.root --inputFiles pythia8CCDIS_10x100_minQ2=100_beamEffects_xAngle=-0.025_hiDiv_1.hepmc

eicrecon
-Pplugins=tracking_test
-Pnthreads=1
-Pjana:debug_plugin_loading=1
-Pjana:nevents=100
-Pjana:timeout=0
-Pdd4hep:xml_files=epic_brycecanyon.xml
-Phistsfile=output_hists.ana.root
input.edm4hep.root

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
