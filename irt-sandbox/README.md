
 * [Installation](#installation)
 * [FRICH example](#frich-example)
 * [pfRICH example](#pfrich-example)


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

FRICH example
-------------

```
# Change to a local 'irt-sandbox' directory in EICrecon repository;
cd ${SANDBOX}/EICrecon/irt-sandbox

# Generate a HEPMC file (presently: 1000 events, p=10 GeV/c, eta=3);
root -l 'hepmc-writer.C("hadron-going-endcap.hepmc", 1000)'

# Run npsim with a simplistic FRICH detector only (in a GEANT Qt mode); use green button to generate one event at a time;
npsim --runType qt --macroFile vis-frich.mac --compactFile $EIC_SHELL_PREFIX/share/epic/epic_frich_only.xml --outputFile ./sim.edm4hep.frich.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.hepmc -N 10

# Run npsim on 1000 events in a batch mode (with FRICH and ePIC tracking detectors);
npsim --runType run --compactFile ../../prefix/share/epic/epic_tracking_and_frich.xml --outputFile ./sim.edm4hep.frich.root --part.userParticleHandler= --inputFiles ./hadron-going-endcap.hepmc -N 1000

# Open simulated ROOT file;
root -l sim.edm4hep.frich.root

# See simulated hits (run these lines in ROOT);
new TCanvas("cv", "", 700, 600);
auto hmap = new TH2D("hmap", "", 100, -300, 300, 100, -150, 450);
events->Project("hmap", "FRICHHits.position.y:FRICHHits.position.x");
gStyle->SetOptStat(0);
hmap->SetMinimum(5);
hmap->Draw("COLZ");

# Run 'eicrecon' and produce output GEANT events ROOT tree in a custom format;
$EIC_SHELL_PREFIX/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files=$EIC_SHELL_PREFIX/share/epic/epic_tracking_and_frich.xml -Ppodio:output_collections="FRICHHits,MCParticles,FRICHTracks,FRICHIrtRadiatorInfo,FRICHIrtParticles,FRICHIrtEvent" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,DRICH,DIRC -Ppodio:output_file="rec.edm4hep.frich.root" sim.edm4hep.frich.root -PFRICH:config=frich-reco.json

# See a digitized hit map;
root -l './frich-hit-map.C("frich-events.root")'

# Run a standalone IRT reconstruction script and inspect 1D output plots;
root -l './frich-reco.C("frich-events.root")'
```

The last command also produced 'frich-optics-with-calibrations.root' file, which contains a copy of the IRT optics dump, yet with the calibrations appended. This file can be used as an input to 'eicrecon', which can also be instructed to run thje IRT reconstruction engine internally (see the first few lines in 
frich-reco-integrated.json).

```
# Run 'eicrecon' calling IRT reconstruction internally, in addition to dumping an event tree for further standalone processing;
$EIC_SHELL_PREFIX/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files=$EIC_SHELL_PREFIX/share/epic/epic_tracking_and_frich.xml -Ppodio:output_collections="FRICHHits,MCParticles,FRICHTracks,FRICHIrtRadiatorInfo,FRICHIrtParticles,FRICHIrtEvent" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,DRICH,DIRC -Ppodio:output_file="rec.edm4hep.frich.root" sim.edm4hep.frich.root -PFRICH:config=frich-reco-integrated.json
```

There is no visualization available (and it is not really needed), but one can see a familiar MINUIT printout at the end of processing, similar to the one produced by frich-reco.C script. This printout would be numerically identical to the one by 'frich-reco.C' script, if one comments lines 'reco->PerformCalibration(200);' and 'reco->ExportModifiedOpticsFile("frich-optics-with-calibrations.root");' in it and re-runs using the same optics file with the built-in calibrations:

```
root -l './frich-reco.C("frich-events.root", "frich-optics-with-calibrations.root")'
```

just because the random numbers will (presently) be initialized the same way as in 'eicrecon'.

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
# Run a geometry overlap check with ePIC tracker;
npsim --runType run --macroFile check-geometry.mac --compactFile $EIC_SHELL_PREFIX/share/epic/epic_tracking_and_pfrich.xml --outputFile ./sim.edm4hep.pfrich.root --part.userParticleHandler= --inputFiles ./electron-going-endcap.hepmc -N 10
```