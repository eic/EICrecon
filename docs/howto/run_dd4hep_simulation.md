# DD4HEP Simulation

## Pythia and other EG

```bash
# Detectors live in
# /opt/detectors
# one can select particular configuration as
# source /opt/detector/epic-22.10.0/setup.sh
#
# or one can set the default detector (now points to epic-nightly)
source /opt/detector/setup.sh

# Run simulation for 1000 events
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --outputFile=sim_output.edm4hep.root --inputFiles mceg.hepmc
```

## Particle gun

There are at least 2 ways of how to run a particle gun:

-   using ddsim command line
-   using geant macro file and invoke GPS


### Using ddsim

Using ddsim (wrapper around ddsim) command line:

```bash
# set the detector
source /opt/detector/setup.sh

# Electrons with 1MeV - 30GeV fired in all directions, 1000 events
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --random.seed 1 --enableGun --gun.particle="e-" --gun.momentumMin 1*MeV --gun.momentumMax 30*GeV --gun.distribution uniform --outputFile gun_sim.edm4hep.root

# Pions from defined position and direction
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=1000 --enableGun --gun.particle="pi-" --gun.position "0.0 0.0 1.0*cm" --gun.direction "1.0 0.0 1.0" --gun.energy 100*GeV --outputFile=test_gun.root

# uniform spread inside an angle:
ddsim --compactFile=$DETECTOR_PATH/epic.xml -N=2 --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile test.root

# run to see all particle gun options
ddsim --help
```

### Using Geant4 macros GPS

[GPS stands for General Particle Source (tutorial)](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/generalParticleSource.html)

GPS is configured in Geant4 macro files. An example of such file
[may be found here](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/macro/gps.mac)

To run ddsim with GPS you have to add [\--enableG4GPS]{.title-ref} flag
and specify Geant4 macro file:

```bash
ddsim --runType run --compactFile=$DETECTOR_PATH/epic.xml --enableG4GPS --macro $DETECTOR_PATH/macro/gps.mac --outputFile gps_example.root
```
