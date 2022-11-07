Full simulation and reco
========================

Quick tutorials
---------------

This is quick tutorial with the steps of how to run sim&recon from
scratch. This work both on BNL and JLab farms as well as personal PCs.

### 0. Install eic-shell

``` {.bash}
> curl https://eicweb.phy.anl.gov/containers/eic_container/-/raw/master/install.sh | bash

# to run eic environment in singularity container
> ./eic-shell
```

More on installing and using singularity here: [Use
singularity](use_singularity.html)

### 1. Convert MCEG to HepMC

The input format should be in HepMC format. If the conversion is needed:
[Convert MCEG](mceg.html)

### 2. Run DD4HEP simulation

Select a detector

``` {.bash}
# Detectors live in 
# /opt/detectors
# one can select particular configuration as
# source /opt/detector/athena-deathvalley-1.5T/setup.sh
#
# or one can set the latest detector
source /opt/detector/setup.sh

# Run simulation
npsim --compactFile=$DETECTOR_PATH/athena.xml --runType=run -N=2 --outputFile=sim_output.edm4hep.root --inputFiles mceg.hepmc
```

### 3. Run Juggler/Gaudi reconstruction

``` {.bash}
#set the same detector as in the simulations
source /opt/detector/setup.sh

export JUGGLER_SIM_FILE=sim_output.edm4hep.root JUGGLER_REC_FILE=rec_output.edm4hep.root JUGGLER_N_EVENTS=10
gaudirun.py /opt/benchmarks/physics_benchmarks/options/reconstruction.py
```

### Particle gun

There are at least 2 ways of running a particle gun out of the box:

-   using npsim command line
-   using geant macro file and invoke GPS

Using npsim (wrapper around ddsim) command line:

``` {.bash}
# Assumed to run from the root of Athena detector repo
# no spread
npsim --compactFile=athena.xml --runType=run -G -N=2 --outputFile=test_gun.root --gun.position "0.0 0.0 1.0*cm" --gun.direction "1.0 0.0 1.0" --gun.energy 100*GeV --part.userParticleHandler='' 

# uniform spread inside an angle:
npsim --compactFile=athena.xml -N=2 --random.seed 1 --enableGun --gun.energy 2*GeV --gun.thetaMin 0*deg --gun.thetaMax 90*deg --gun.distribution uniform --outputFile test.root
```

Using GPS

[General Particle Source
tutorial](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/generalParticleSource.html)

GPS is configured in Geant4 macro files. An example of such file [may be
found
here](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/macro/gps.mac)

To run npsim with GPS you have to add [\--enableG4GPS]{.title-ref} flag
and specify Geant4 macro file:

``` {.bash}
npsim --runType run --compactFile athena.xml --enableG4GPS --macro macro/gps.mac --outputFile gps_example.root
```

### Geometry visualization

There are many ways to see the geometry and tracks:

1.  Through Geant4 event display
2.  geoDisplay (root geoViewer)
3.  ddeve (root EVE based event display\...)
4.  dd\_web\_display (using browser and jsroot library)

To run Geant4 event display:

``` {.bash}
# Assumed to run from the root of Athena detector repo
npsim --runType vis --compactFile athena.xml --macro macro/vis.mac --outputFile test.root --enableG4GPS --enableQtUI
```

Geometry conversion
-------------------

### Convert to GDML

There is a convert\_to\_gdml.py script in the detector repository
(<https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/scripts/convert_to_gdml.py>).
That can be used to export ALL of ATHENA to gdml, but not individual
detector systems.

This is actually
[done](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/blob/master/.gitlab-ci.yml#L168)
on every commit and the results are saved as job artifacts.

[The latest athena.gdml from the master
branch](https://eicweb.phy.anl.gov/api/v4/projects/473/jobs/artifacts/master/raw/geo/athena.gdml?job=report&item=default)

### Convert to root

One can use dd\_web\_display to actually just save root geometry

``` {.bash}
dd_web_display --export athena.xml  # will create a .root file with the geometry
```

How XML is invoked from C++ in DD4Hep
-------------------------------------

\> How does the XML file in the compact directory know which c++ file to
load when we include the respective XML file in the main athena.xml
file?

1.  Xml compact files has [\<detector \...\>\</detector\>]{.title-ref}
    tag which has [type]{.title-ref} attribute that tells which C++ type
    to use:

    ``` {.xml}
    <detector id="ForwardRICH_ID" name="DRICH" type="athena_DRICH" ... >
    ```

2.  C++ file usually has [createDetector(\...)]{.title-ref} function and
    [DECLARE\_DETELEMENT]{.title-ref} macro which binds the function to
    the name used in xml [\<detector\>]{.title-ref} type attribute. C++
    code looks like this:

    ``` {.c++}
    static Ref_t createDetector(Detector& desc, xml::Handle_t handle, SensitiveDetector sens) {
        // ...
    }

    DECLARE_DETELEMENT(athena_DRICH, createDetector)
    ```

3.  How DD4Hep finds and loads compiled components?

    \> This is going into technical details which users usually don\'t
    need. Installation paths and environment variables are set by
    container/spack and should work out of the box.

    DD4Hep uses modular plugin mechanism to load C++ code. When athena
    C++ is compiled, two files are created:

    -   [athena.components]{.title-ref} - a text file stating what
        components one can find in athena.so. From our example, there
        will be a record like
        [v2:libathena.so:athena\_DRICH]{.title-ref} among other records.
    -   [libathena.so]{.title-ref} - compiled C++ library with all
        detectors from athena repo

    So when the type of the detector is given, like
    [athena\_DRICH]{.title-ref}. DD4Hep uses
    [LD\_LIBRARY\_PATH]{.title-ref} to look through .components files
    and then figures out what .so file to load to get the correct C++
    code executed.
