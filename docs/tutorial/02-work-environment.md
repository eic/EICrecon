
| title                                             | teaching | exercises | questions                                                                      | objectives                                                                                  | keypoints                                                                                            |
|---------------------------------------------------|----------|-----------|--------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------|
| Work Environment for EPIC Reconstruction Software | 10       | 10        | How to setup a work environment to use or develop EIC reconstruction software? | Clone EICrecon repository and build with eic-shell. <br/><br/>Obtain a simulated data file. | Use eicrecon executable to run reconstruction on a podio input file and to create podio output file. |



## How do I setup a work environment for EICrecon?

A "work environment" includes a working directory and a full software stack that includes all dependencies for
EICrecon. It also includes the EICrecon code itself. By far, the easiest way to set this up is using eic-shell
as outlined in the first tutorial. Start with creating a work directory and then getting eic-shell up and running.


A quick summary (please see first tutorial for details):

```console
mkdir ~/eic
cd ~/eic

curl --location https://get.epic-eic.org | bash
./eic-shell

# or, if /cvmfs is available:
# n.b. on JLab ifarm you may need to do 'module load singularity/3.9.5' first

singularity exec /cvmfs/singularity.opensciencegrid.org/eicweb/jug_xl:nightly eic-shell

```
Once inside the eic-shell, you should source the geometry setup script since this is not done by default.

```
source /opt/detector/setup.sh
```

Next, clone the EICrecon repository. In the future, you may want to work with a prebuilt EICrecon that comes
with eic-shell. For now though, it is better to just clone the whole repository so you can modify it and
submit changes back to GitHub.

```console
git clone https://github.com/eic/EICrecon

# or, if you have ssh keys set up on github

git clone git@github.com:eic/EICrecon
```

Check that you can build EICrecon using the packages in eic-shell:

```console
cd EICrecon
cmake -S . -B build
cmake --build build --target install -- -j8
```

If you are not familiar with cmake, the first command above (cmake -S . -B build) will create a directory "build"
and place files there to drive the build of the project in the source directory "." (i.e. the current dirctory).
The second cmake command (cmake --build build --target install -- -j8) actually performs the build and installs
the compiled plugins, exectuables, etc. Note that the "-j8" option tells it to use 8 threads to compile. If you
have more cores available, then set this number to an appropriate value.

### Exercise 1:

Use the commands above to setup your working directory and build *EICrecon*.


## How do I run *eicrecon*?

*eicrecon* is the main reconstruction executable. To run it though, you should add it to your PATH and set up
any other environment parameters needed. Do this by sourcing the "eicrecon-this.sh" file that should have been
created and installed in the previous step. Once you have done that, run eicrecon with no arguments to see that
it is found and prints the usage statement.

```console
source ~/eic/EICrecon/bin/eicrecon-this.sh
eicrecon

Usage:
    eicrecon [options] source1 source2 ...

Description:
    Command-line interface for running JANA plugins. This can be used to
    read in events and process them. Command-line flags control configuration
    while additional arguments denote input files, which are to be loaded and
    processed by the appropriate EventSource plugin.

Options:
   -h   --help                  Display this message
   -v   --version               Display version information
   -c   --configs               Display configuration parameters
   -l   --loadconfigs <file>    Load configuration parameters from file
   -d   --dumpconfigs <file>    Dump configuration parameters to file
   -b   --benchmark             Run in benchmark mode
   -Pkey=value                  Specify a configuration parameter

Example:
    jana -Pplugins=plugin1,plugin2,plugin3 -Pnthreads=8 inputfile1.txt


-----------
    eicrecon parameters: (specify with -Pparam=value)

        -Phistsfile=file.root    Set name for histograms/trees produced by plugins
```

The usage statement gives several command line options. Two of the most important ones are the
*"-l"* and *"-Pkey=value"* options. Both of these allow you to set *configuration parameters*
in the job. These are how you can modify the behavior of the job. Configuration parameters
will pretty much always have default values set by algorithm authors so it is often not necessary
to set this yourself. If you need to though, these are how you do it. Use the "-Pkey=value"
form if you want to set the value directly on the command line. You may pass mutiple options like
this. The "-l" option is used to specify a configuration file where you may set a large number
of values. The file format is one parameter per line with one or more spaces separating the
configuration parameter name and its value. Empty lines are OK and "#" can be used to specify
comments.


## Get a simulated data file
The third tutorial in this series described how to generate a simulated data file. If you
followed the exercises in that tutorial you can use a file you generated there. If not, then
you can grab an existing simulted data file for mthe web for the purposes of this tutorial.
This will grab a 226MB file:

```console
wget https://eicaidata.s3.amazonaws.com/2022-09-26_ncdis10x100_minq2-1_100ev.edm4hep.root
```

### Exercise 2:

Run `eicrecon` over your simulated data file by giving it as an argument to the program.
e.g.

```console
eicrecon 2022-09-26_ncdis10x100_minq2-1_100ev.edm4hep.root
```


## Generating a podio output file
To write reconstructed values to an output file, you need to tell *eicrecon* what to write.
There are several options available, but the mosrt useful one is *podio:output_include_collections*.
This is a comma separated list of colelctions to write to the output file. For example:

```console
eicrecon -Ppodio:output_include_collections=ReconstructedParticles 2022-09-26_ncdis10x100_minq2-1_100ev.edm4hep.root
```

To see a list of possible collections, run *eicrecon -L* .

### Exercise 3

Use *eicrecon* to generate an output file with both *ReconstructedParticles* and *EcalEndcapNRawHits*



{% include links.md %}
