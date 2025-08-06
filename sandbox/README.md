
```
#
# Define installation area (/DATA00/ayk/ePIC in this line by line example below);
# start the container; define environment variables
#
export SANDBOX=/DATA00/ayk/ePIC

cd $SANDBOX

# curl --location https://get.epic-eic.org | bash

./eic-shell

. environ.sh
```

```
#
# Install EDM4eic, IRT 2.0, epic & EICrecon
#
# '-b pfrich': IRT 2.0;
git clone -b irt-2.0 https://github.com/eic/EDM4eic.git
cmake -S EDM4eic -B EDM4eic/build -DBUILD_DATA_MODEL=ON -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build EDM4eic/build -j8
cmake --install EDM4eic/build

git clone -b irt-2.0 https://github.com/eic/irt.git
cmake -S irt -B irt/build -DCMAKE_BUILD_TYPE=Debug -DDELPHES=OFF -DEVALUATION=OFF -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build irt/build -j8
cmake --install irt/build

git clone -b irt-2.0 https://github.com/eic/epic.git
cmake -S epic -B epic/build -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build epic/build -j8
cmake --install epic/build

git clone -b irt-2.0 https://github.com/eic/EICrecon.git
cmake -S EICrecon -B EICrecon/build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_FIND_DEBUG_MODE=OFF -DCMAKE_INSTALL_PREFIX=$EIC_SHELL_PREFIX -Wno-dev
cmake --build EICrecon/build -j8
cmake --install EICrecon/build
```

```
#
# Adjust environment variables (FIXME: pass SANDBOX variable from the upstream environment);
#
export SANDBOX=/DATA00/ayk/ePIC
export LD_LIBRARY_PATH=${SANDBOX}/prefix/lib:${LD_LIBRARY_PATH}
export DETECTOR_PATH=${SANDBOX}/prefix/share/epic
export JANA_PLUGIN_PATH=${SANDBOX}/prefix/lib/EICrecon/plugins:$JANA_PLUGIN_PATH
export ROOT_LIBRARY_PATH=${SANDBOX}/prefix/lib:${ROOT_LIBRARY_PATH}
```

```
#
# The rest is for a forward dummy RICH (FRICH) detector. Replace "FRICH" by either "BRICH" (backward dummy RICH) or "PFRICH" if needed.  
#
```

```
cd ${SANDBOX}/EICrecon/sandbox

#
# Create a HEPMC3 file with 1000 events; NB: check the actual script that it is up to date
# (eta range in particular)!;
#
root -l 'hepmc-writer.C("hadron-endcap.hepmc", 1000)'

#
# Vizualize FRICH detector only; geometry check: /geometry/test/run FRICH_0; green button: new event
#
npsim --runType qt --macroFile vis-frich.mac --compactFile ../../prefix/share/epic/epic_frich_only.xml --outputFile ./sim.edm4hep.dummy.root --part.userParticleHandler= --inputFiles ./hadron-endcap.hepmc -N 10

#
# Perform GEANT4 pass; 
#
npsim --runType run --compactFile ../../prefix/share/epic/epic_tracking_and_frich.xml --outputFile ./sim.edm4hep.frich.root --part.userParticleHandler= --inputFiles ./hadron-endcap.hepmc -N 1000

#
# See simulated hits
#
root -l sim.edm4hep.frich.root
root [1] events->Draw("FRICHHits.position.y:FRICHHits.position.x")
```

```
#
# Run EICrecon
#
$SANDBOX/prefix/bin/eicrecon -Pplugins="janadot" -Pdd4hep:xml_files="../../prefix/share/epic/epic_tracking_and_frich.xml" -Ppodio:output_collections="FRICHHits,MCParticles,FRICHTracks,FRICHIrtDebugInfoTables" -Peicrecon:LogLevel="info" -Pjana:nevents="0" -Pjana:debug_plugin_loading="1" -Pacts:MaterialMap="calibrations/materials-map.cbor" -Pplugins_to_ignore=LUMISPECCAL,LOWQ2,FOFFMTRK,RPOTS,B0TRK,ZDC,B0ECAL,FHCAL,BHCAL,EHCAL,FEMC,BEMC,EEMC,DRICH,DIRC -Ppodio:output_file="rec.edm4hep.frich.root" sim.edm4hep.frich.root -PFRICH:config=frich-reco.json

#
# See digitized hit map
#
root -l './frich-hit-map.C("frich-events.root")'

#
# Run standalone IRT 2.0 reconstruction script
#
root -l './frich-reco.C("frich-events.root")'
```