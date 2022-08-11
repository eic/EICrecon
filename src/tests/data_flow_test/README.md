This is guinea pig plugin for active development phase

```bash

# uniform spread inside an angle:

ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2000 --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile tracking_test_gun.root
```