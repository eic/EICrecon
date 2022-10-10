## Jana2 flags 


- **plugins** - Comma separated list of additional plugins to load. 
  Note that:
  1. eicrecon has a list of default plugins that are required for full simulation and loaded automatically
  2. If a plugin is loaded, it doesn't mean that its factories are activated. Factories are activated based on required data 
  
- **nthreads** - number of threads. Currently whould be 1
- **jana:debug_plugin_loading** - if 1, prints additional information about plugin loading process. Convenient to check that all plugins are found and correctly loaded.
- **jana:nevents** - number of events to calculate  
- **jana:nskip** - number of first events to ckip
- **jana:timeout** - controlls jana self-check ticks. ```-Pjana:timeout=0``` switches off 
  JANA self-check ticks, which is useful if you would like to use debugger breakpoints to 
  pause code execution. Without ```-Pjana:timeout=0``` jana stops after a long pause in debugger
- **jana:debug_mode** - ??? 



Example: 

```bash
eicrecon

# add additional plugins to execution
-Pplugins=tracking_efficiency,       # include additional plugins
-Pnthreads=1                         # run in 1 thread
-Pjana:debug_plugin_loading=1        # print more info on plugins loading
-Pjana:nskip=500                     # skip first 500 events
-Pjana:nevents=1000                  # process 1000 events
-Pjana:debug_mode=1                  #  
-Pjana:timeout=0                     # Can now pause in IDE debugger
input_file.edm4hep.root              # Input file
```
