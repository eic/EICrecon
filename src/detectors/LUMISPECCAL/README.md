To run this plugin with tracking

```bash
eicrecon
-Pplugins=tracking_occupancy,tracking_efficiency
-Pnthreads=1
-Pjana:debug_plugin_loading=1
-Pjana:nevents=100
-Pjana:timeout=0
-Ptracking_efficiency:LogLevel=info
-PTracking:CentralTrackerSourceLinker:LogLevel=info
-PCKFTracking:TrackingResultTrajectory:LogLevel=info
-Ptracking_efficiency:LogLevel=debug
-Ppodio:output_file=/home/romanov/work/data/eicrecon_test/tracking_test_gun.edm4eic.root
-Pdd4hep:xml_files=/home/romanov/eic/soft/detector/main/compiled/epic/share/epic/epic_tracking_only.xml
-Phistsfile=/home/romanov/work/data/eicrecon_test/tracking_test_gun.ana.root
/home/romanov/work/data/eicrecon_test/output.edm4hep.root
```

Flags explained:
```bash
#This flag lists plugins(submodules) needed to run tracking reconstruction and analysis
-Pplugins=acts,tracking,BTRK,ECTRK,BVTX,MPGD,tracking_occupancy,tracking_efficiency

# Number of parallel threads. Currently only 1 works. 
# It is a limitation of an event model IO and will be fixed later 
-Pnthreads=1

# Write exactly what happens when plugins are loading. Good during debugging time.  
-Pjana:debug_plugin_loading=1

# Removes self "hang" watchdog. 
# Needed during debugging if you pause code execution with breakpoints
-Pjana:timeout=0


# Number of events to process. 
# (If events needs to be skipped there is also -Pjana:nskip flag) 
-Pjana:nevents=100

# xxx:LogLevel - various plugins/factories logging levels
# trace, debug, info, warn, error, critical, off:
# trace    - something very verbose like each hit parameter
# debug    - all information that is relevant for an expert to debug but should not be present in production
# info     - something that will always (almost) get into the global log
# warning  - something that needs attention but results are likely usable
# error    - something bad that makes results probably unusable
# critical - imminent software failure and termination
# Logging explained here: 
https://github.com/eic/EICrecon/blob/main/docs/Logging.md


# DD4Hep xml file for the detector describing the geometry. 
# Can be set by this flag or env. variables combinations: ${DETECTOR_PATH}/${DETECTOR}.xml
-Pdd4hep:xml_files=...

# Example 1: full path to detector.xml
-Pdd4hep:xml_files=/path/to/dd4hep/epic/epic_tracking_only.xml

# Example2: DETECTOR_PATH env var is set in eic_shell, so it could be 
-Pdd4hep:xml_files=${DETECTOR_PATH}/epic_tracking_only.xml


# Alternatively it could be set through environment variables to not to add -Pdd4hep:xml_files every run
# Then -Pdd4hep:xml_files flag is not needed. (!) Note that ".xml" is not needed in ${DETECTOR}
export DETECTOR_PATH="/path/to/dd4hep/epic/"
export DETECTOR="epic_tracking_only"

# This makes tracking output data and input MC particles to be written to the output
-Ppodio:output_include_collections="ReconstructedParticles,TrackParameters,MCParticles"

# There is a centralized file where plugins can save their histograms:
-Phistsfile=/home/romanov/work/data/eicrecon_test/tracking_test_gun.ana.root

# all filenames that doesn't start with -<flag> are interpreted as input files
# So this is an input file path
/home/romanov/work/data/eicrecon_test/output.edm4hep.root
```

### Saving collections to a file

In order to save podio data to the file one has to provide 2 parameters
defining a list of objects and the output file name:

```bash
# This makes tracking output data and input MC particles to be written to the output
-Ppodio:output_include_collections="ReconstructedParticles,TrackParameters,MCParticles"

# This sets file path containing output tree
-Ppodio:output_file=/home/romanov/work/data/eicrecon_test/tracking_test_gun.edm4eic.root
```

One can see the list of data names, when eicrecon is running:

```
  FACTORIES
  ----------------------------------------------------------------------------------
    Plugin                Object name                               Tag                
  -----------  -------------------------------------  ------------------------------  
  ...            
  tracking.so  edm4eic::TrackParameters               TrackParameters                 
  tracking.so  edm4eic::ReconstructedParticle         ReconstructedParticles          
  ...                                                 ^
                                                      this name
```

