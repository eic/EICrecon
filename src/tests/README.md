
This Readme is temporary. Contains some files that might be used for debugging/testing


## 15 August 2022

- Podio 0.15 
- edm4hep 0.6 
- Files with electrons 0-30GeV fired in all directions


https://eicaidata.s3.amazonaws.com/2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root

https://eicaidata.s3.amazonaws.com/2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_2k.edm4hep.root

https://eicaidata.s3.amazonaws.com/2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_10k.edm4hep.root



Files are simulated with:

```bash
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_1k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_2k.edm4hep.root
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=10000 --random.seed 1 --enableGun  --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile 2022-08-15_pgun_e-_podio-0.15_edm4hep-0.6_0-30GeV_alldir_10k.edm4hep.root
```


