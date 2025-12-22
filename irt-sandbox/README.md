
 * [Installation](#installation)
 * [dRICH example](#drich-example)


Installation
------------

```
# Change to your local working directory;
cd <your-working-directory>

# Install eic-shell script and a docker container image known to work;
#curl -L https://github.com/eic/eic-shell/raw/main/install.sh     | bash -s --  --version 25.07.0-stable --no-cvmfs
curl --location https://get.epic-eic.org | bash

# Run 'eic-shell';
./eic-shell

# Use git branch irt-2.1c for all repositories;
export branch="irt-2.1c"

# Download EICrecon;
git clone -b ${branch} https://github.com/eic/EICrecon.git

# Run a sandbox environment script;
. EICrecon/irt-sandbox/environ.sh

# Install EDM4eic;
git clone -b ${branch} https://github.com/eic/EDM4eic.git
cmake -S EDM4eic -B EDM4eic/build -DBUILD_DATA_MODEL=ON -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build EDM4eic/build -j8
cmake --install EDM4eic/build

# Install IRT;
git clone -b ${branch} https://github.com/eic/irt.git
cmake -S irt -B irt/build -DCMAKE_BUILD_TYPE=Debug -DDELPHES=OFF -DEVALUATION=OFF -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -DJSON_EXPORT=ON -Wno-dev
cmake --build irt/build -j8
cmake --install irt/build

# Install epic;
git clone -b  ${branch} https://github.com/eic/epic.git
cmake -S epic -B epic/build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -DWITH_IRT=YES -Wno-dev
cmake --build epic/build -j8
cmake --install epic/build

#Install EICrecon;
cmake -S EICrecon -B EICrecon/build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_FIND_DEBUG_MODE=OFF -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build EICrecon/build -j8
cmake --install EICrecon/build
```

dRICH example
-------------

```
# Change to a local 'irt-sandbox' directory in EICrecon repository;
cd ${SANDBOX}/EICrecon/irt-sandbox

# Generate a HEPMC file (here: 1000 events, pions, p=10 GeV/c, eta=2.0, phi=30 degrees);
root -l 'hepmc-writer-single-track.C("hadron-going-endcap.hepmc", 1000, 211, 10.0, 10.0, 2.0, 2.0, M_PI/6, M_PI/6)'

# Run npsim with a dRICH detector only (in a GEANT Qt mode); use green button to generate one event at a time;
npsim --runType qt --macroFile vis-drich.mac --compactFile $EIC_SHELL_PREFIX/share/epic/epic_drich_only.xml --outputFile ./sim.edm4hep.drich.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.hepmc -N 10

# Run npsim on 1000 events in a batch mode (with dRICH only);
npsim --runType run --compactFile $EIC_SHELL_PREFIX/share/epic/epic_drich_only.xml --outputFile ./sim.edm4hep.drich.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.hepmc -N 1000

# Open simulated ROOT file;
root -l sim.edm4hep.drich.root

# See simulated hits;
root [1] events->Draw("DRICHHits.position.y:DRICHHits.position.x");
```

```
# Generate another HEPMC file (here: 10000 events, pions, p=10 GeV/c, eta=[1.5 .. 3.5], phi=30 degrees);
root -l 'hepmc-writer-single-track.C("hadron-going-endcap.calibration.hepmc", 10000, 211, 10.0, 10.0, 1.5, 3.5, M_PI/6, M_PI/6)'

# Run npsim on 10000 events in a batch mode (with DRICH and ePIC tracking detectors) in a eta [1.5 .. 3.5] range;
npsim --runType run --compactFile ../../prefix/share/epic/epic_tracking_and_drich.xml --outputFile ./sim.edm4hep.drich.calibration.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.calibration.hepmc -N 10000

# eicrecon dry run -> produce output event ROOT tree in a custom format for a standalone calibration script;
# NB: make sure "IntegratedReconstruction": "no" in drich-reco.json file;
$EIC_SHELL_PREFIX/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files=$EIC_SHELL_PREFIX/share/epic/epic_tracking_and_drich.xml -Ppodio:output_collections="DRICHHits,MCParticles,DRICHTracks,DRICHIrtRadiatorInfo,DRICHIrtParticles,DRICHIrtEvent" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,DRICH,DIRC -Ppodio:output_file="rec.edm4hep.drich.calibration.root" sim.edm4hep.drich.calibration.root -PDRICH:config=drich-reco.json

# Produce drich-calibration.json file for future use in the eicrecon integrated pass;
root -l './drich-calibration.C("drich-events.calibration.root", "drich-calibration.json")'
```

```
# Run npsim on 1000 events on the first produced .hepmc file in a batch mode (with DRICH and ePIC tracking detectors);
npsim --runType run --compactFile ../../prefix/share/epic/epic_tracking_and_drich.xml --outputFile ./sim.edm4hep.drich.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.hepmc -N 1000

# Run 'eicrecon' with IRT2 engine activated; NB: make sure "IntegratedReconstruction": "yes" in drich-reco.json file;
# "WriteOutputTree" may be changed to "no" to save disk space; either tune "CombinedEvaluationPlotsGeometry" and "evaluation-plots-geometry" to fit on your
# screen or disable graphics output at the end of processing (change all "display" keys to "store" in drich-reco.json) in case of problems; 
$EIC_SHELL_PREFIX/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files=$EIC_SHELL_PREFIX/share/epic/epic_tracking_and_drich.xml -Ppodio:output_collections="DRICHHits,MCParticles,DRICHTracks,DRICHIrtRadiatorInfo,DRICHIrtParticles,DRICHIrtEvent" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,DRICH,DIRC -Ppodio:output_file="rec.edm4hep.drich.root" sim.edm4hep.drich.root -PDRICH:config=drich-reco.json

# Re-read the output canvases by hand from the produced tree;
root -l drich-events.root
root [2] cx->Draw(); ca->Draw(); cg->Draw();

# PODIO output parser is not available as of yet;
```