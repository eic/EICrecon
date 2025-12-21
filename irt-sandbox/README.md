
 * [Installation](#installation)
 * [pfRICH example](#pfrich-example)
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

# Use git branch irt-2.1b for all repositories;
export branch="irt-2.1b"

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
cmake -S irt -B irt/build -DCMAKE_BUILD_TYPE=Debug -DDELPHES=OFF -DEVALUATION=OFF -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
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

pfRICH example
--------------

```
# Change to a local 'irt-sandbox' directory in EICrecon repository;
cd ${SANDBOX}/EICrecon/sandbox

# Generate a HEPMC file (here: 1000 events, pions, p=7 GeV/c, eta=-2.5, phi=0);
root -l 'hepmc-writer-single-track.C("electron-going-endcap.hepmc", 1000, 211, 7.0, 7.0, -2.5, -2.5, 0.0, 0.0)'

# Run npsim with a pfRICH detector only (in a GEANT Qt mode); use green button to generate one event at a time;
npsim --runType qt --macroFile vis-pfrich.mac --compactFile $EIC_SHELL_PREFIX/share/epic/epic_pfrich_only.xml --outputFile ./sim.edm4hep.pfrich.root --part.userParticleHandler= --inputFiles ./electron-going-endcap.hepmc -N 10

# Run npsim on 1000 events in a batch mode (with pfRICH only);
npsim --runType run --compactFile $EIC_SHELL_PREFIX/share/epic/epic_pfrich_only.xml --outputFile ./sim.edm4hep.pfrich.root --part.userParticleHandler= --inputFiles ./electron-going-endcap.hepmc -N 1000

# Open simulated ROOT file;
root -l sim.edm4hep.pfrich.root

# See simulated hits;
root [1] events->Draw("PFRICHHits.position.y:PFRICHHits.position.x");
```

```
# Run a geometry overlap check with ePIC tracker (takes several minutes);
npsim --runType run --macroFile check-geometry.mac --compactFile $EIC_SHELL_PREFIX/share/epic/epic_tracking_and_pfrich.xml --outputFile ./sim.edm4hep.pfrich.root --part.userParticleHandler= --inputFiles ./electron-going-endcap.hepmc -N 10
```

```
# Run npsim on 1000 events in a batch mode (with pfRICH and ePIC tracking detectors);
npsim --runType run --compactFile ../../prefix/share/epic/epic_tracking_and_pfrich.xml --outputFile ./sim.edm4hep.pfrich.root --part.userParticleHandler= --inputFiles ./electron-going-endcap.hepmc -N 1000

# Run 'eicrecon' and produce output GEANT events ROOT tree in a custom format;
$EIC_SHELL_PREFIX/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files=$EIC_SHELL_PREFIX/share/epic/epic_tracking_and_pfrich.xml -Ppodio:output_collections="PFRICHHits,MCParticles,PFRICHTracks,PFRICHIrtRadiatorInfo,PFRICHIrtParticles,PFRICHIrtEvent" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,PFRICH,DIRC -Ppodio:output_file="rec.edm4hep.pfrich.root" sim.edm4hep.pfrich.root -PPFRICH:config=pfrich-reco.json

# See a digitized hit map;
root -l './pfrich-hit-map.C("pfrich-events.root")'

# Run a standalone IRT reconstruction script and inspect 1D output plots;
root -l './pfrich-reco.C("pfrich-events.root")'
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

# Run npsim on 1000 events in a batch mode (with DRICH and ePIC tracking detectors);
npsim --runType run --compactFile ../../prefix/share/epic/epic_tracking_and_drich.xml --outputFile ./sim.edm4hep.drich.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.hepmc -N 1000

# Run 'eicrecon' and produce output GEANT events ROOT tree in a custom format;
$EIC_SHELL_PREFIX/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files=$EIC_SHELL_PREFIX/share/epic/epic_tracking_and_drich.xml -Ppodio:output_collections="DRICHHits,MCParticles,DRICHTracks,DRICHIrtRadiatorInfo,DRICHIrtParticles,DRICHIrtEvent" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,DRICH,DIRC -Ppodio:output_file="rec.edm4hep.drich.root" sim.edm4hep.drich.root -PDRICH:config=drich-reco.json

# See a digitized hit map;
root -l './drich-hit-map.C("drich-events.root")'

# Run a standalone IRT reconstruction script and inspect 1D output plots;
root -l './drich-reco.C("drich-events.root")'
```