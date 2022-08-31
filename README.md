# EICrecon
EIC Reconstruction - JANA based

## Build Instructions
These are temporary build instructions as the build system and environment
setup system needs to be identified. These instructions include building
all of the dependencies manually. 

Start by setting the EICTOPDIR environment variable. This makes it easier
to reference directories in the instructions below. Set this to a
directory where you want to build and keep the software. If you wish to
use your current directory then just do this:
~~~
export EICTOPDIR=${PWD}
~~~

### Python Environment
PODIO requires that the python packages _pyyaml_ and _jinja2_ be installed. 
If these are not already installed on your system then you can do so either
at a system level (requires sudo privilege) or just create a virtual environment:

~~~
mkdir -p ${EICTOPDIR}/python/virtual_environments
python3 -m venv ${EICTOPDIR}/python/virtual_environments/venv
source ${EICTOPDIR}/python/virtual_environments/venv/bin/activate
pip install pyyaml jinja2
~~~

### boost
Make sure [boost](https://www.boost.org/) is installed (needed for DD4hep).
On macosx 12.4 I did this with:
~~~
brew install boost
~~~

On RHEL7.9 the version installed via yum was too old so I did it from source
like this.
~~~
export BOOST_VERSION=boost-1.79.0
mkdir -p ${EICTOPDIR}/BOOST
cd ${EICTOPDIR}/BOOST
export Boost_ROOT=${EICTOPDIR}/BOOST/${BOOST_VERSION}/installed
git clone --recursive https://github.com/boostorg/boost.git -b ${BOOST_VERSION} ${BOOST_VERSION}
cmake3 -S ${BOOST_VERSION} -B build  -DCMAKE_INSTALL_PREFIX=${Boost_ROOT} -DCMAKE_CXX_STANDARD=17
cmake3 --build build --target install -- -j8
~~~

### ROOT
We need a modern root version built using the C++17 standard. You may obtain
this in a number of ways for your system, but here is how it may be built
from source. Be aware that this is the longest step in the whole produce
since it may take several minutes (maybe much more). Adjust the number of
threads in the _make_ command to match what is available on your system to
speed it up.
~~~
mkdir ${EICTOPDIR}/root
cd ${EICTOPDIR}/root
wget https://root.cern/download/root_v6.26.04.source.tar.gz
tar xzf root_v6.26.04.source.tar.gz
mv root-6.26.04 root-6.26.04.src
cmake3 -S root-6.26.04.src -B root-6.26.04.build -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=root-6.26.04 -Dbuiltin_glew=ON
cmake3 --build root-6.26.04.build --target install -- -j8

source ${EICTOPDIR}/root/root-6.26.04/bin/thisroot.sh
~~~

### JANA
~~~
export JANA_VERSION=v2.0.7
export JANA_HOME=${EICTOPDIR}/JANA/${JANA_VERSION}
git clone https://github.com/JeffersonLab/JANA2 -b ${JANA_VERSION} ${JANA_HOME}
cd ${JANA_HOME}
cmake3 -S . -B build -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=${JANA_HOME} -DUSE_ROOT=1 # -DUSE_ZEROMQ=1 -DUSE_PYTHON=1
cmake3 --build build --target install -- -j8
source ${JANA_HOME}/bin/jana-this.sh                # Set environment to use this
~~~

### spdlog
~~~
export SPDLOG_VERSION=v1.10.0
export SPDLOG_HOME=${EICTOPDIR}/spdlog/${SPDLOG_VERSION}
export spdlog_ROOT=${SPDLOG_HOME}/install
git clone https://github.com/gabime/spdlog -b ${SPDLOG_VERSION} ${SPDLOG_HOME}
cd ${SPDLOG_HOME}
cmake3 -S . -B build -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=${spdlog_ROOT}
cmake3 --build build --target install -- -j8
~~~

git clone https://github.com/gabime/spdlog.git

### PODIO
~~~
export PODIO_VERSION=v00-14-03
export PODIO_HOME=${EICTOPDIR}/PODIO/${PODIO_VERSION}
export PODIO=${PODIO_HOME}/install
export podio_ROOT=${PODIO}
git clone https://github.com/AIDASoft/podio -b ${PODIO_VERSION} ${PODIO_HOME}
cd ${PODIO_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${PODIO} -DCMAKE_CXX_STANDARD=17 -DUSE_EXTERNAL_CATCH2=OFF
cmake3 --build build --target install -- -j8
source ${PODIO_HOME}/env.sh
~~~

### EDM4hep
~~~
export EDM4HEP_VERSION=v00-06
export EDM4HEP_HOME=${EICTOPDIR}/EDM4hep/${EDM4HEP_VERSION}
export EDM4HEP=${EDM4HEP_HOME}/install 
export EDM4HEP_ROOT=${EDM4HEP}
git clone https://github.com/key4hep/EDM4hep -b ${EDM4HEP_VERSION} ${EDM4HEP_HOME}
cd ${EDM4HEP_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${EDM4HEP} -DCMAKE_CXX_STANDARD=17 -DUSE_EXTERNAL_CATCH2=OFF
cmake3 --build build --target install -- -j8
~~~

### eicd
~~~
export EICD_VERSION=v2.0.0
export EICD_HOME=${EICTOPDIR}/eicd/${EICD_VERSION} 
export EICD_ROOT=${EICD_HOME}/install 
git clone https://github.com/eic/eicd -b ${EICD_VERSION} ${EICD_HOME}
cd ${EICD_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${EICD_ROOT} -DCMAKE_CXX_STANDARD=17
cmake3 --build build --target install -- -j8
~~~

### DD4hep
These instructions build DD4hep without Geant4 support. The reconstruction
framework does not otherwise require Geant4 so we avoid including support
for it for now.
~~~
export DD4HEP_VERSION=v01-20-02
export DD4HEP_HOME=${EICTOPDIR}/DD4hep/${DD4HEP_VERSION}
git clone https://github.com/AIDASoft/DD4hep -b ${DD4HEP_VERSION} ${DD4HEP_HOME}
cd ${DD4HEP_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${DD4HEP_HOME}/install -DCMAKE_CXX_STANDARD=17 -DBUILD_DOCS=OFF -DBoost_NO_BOOST_CMAKE=ON -DROOT_DIR=$ROOTSYS
cmake3 --build build --target install -- -j8
source ${DD4HEP_HOME}/install/bin/thisdd4hep.sh
~~~

### ACTS
~~~
export EIGEN_VERSION=3.4.0
export EIGEN_HOME=${EICTOPDIR}/EIGEN/${EIGEN_VERSION}
git clone https://gitlab.com/libeigen/eigen.git -b ${EIGEN_VERSION} ${EIGEN_HOME}
cd ${EIGEN_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${EIGEN_HOME} -DCMAKE_CXX_STANDARD=17
cmake3 --build build --target install -- -j8
export Eigen3_ROOT=${EIGEN_HOME}

export ACTS_VERSION=v19.4.0
export ACTS_HOME=${EICTOPDIR}/ACTS/${ACTS_VERSION}
git clone https://github.com/acts-project/acts -b ${ACTS_VERSION} ${ACTS_HOME}
cd ${ACTS_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${ACTS_HOME}/install -DCMAKE_CXX_STANDARD=17 -DACTS_BUILD_PLUGIN_DD4HEP=on -DACTS_BUILD_PLUGIN_TGEO=on
cmake3 --build build --target install -- -j8
source ${ACTS_HOME}/install/bin/this_acts.sh
~~~

### Detector Geometry
The detector geometry itself is contained in separate repositories.
The _EPIC_ reference detector design is in a repository
located [here](https://github.com/eic/epic). This requires at least _ACTS_
and the _{fmt}_ package the latter of which is built in the instructions here.

Note: These instructions turn off the requirement of the DDG4 component in both the
_ip6_ and _epic_ geometries since it requires GEANT4 which is not needed here.

~~~
mkdir -p ${EICTOPDIR}/detectors
cd ${EICTOPDIR}/detectors

export FMT_VERSION=9.0.0
mkdir -p ${EICTOPDIR}/detectors/fmt
cd ${EICTOPDIR}/detectors/fmt
export fmt_ROOT=${EICTOPDIR}/detectors/fmt/${FMT_VERSION}/install
export LD_LIBRARY_PATH=${EICTOPDIR}/detectors/fmt/${FMT_VERSION}/install/lib64:${EICTOPDIR}/detectors/fmt/${FMT_VERSION}/install/lib:${LD_LIBRARY_PATH}
git clone https://github.com/fmtlib/fmt -b ${FMT_VERSION} ${FMT_VERSION}
cmake3 -S ${FMT_VERSION} -B build  -DCMAKE_INSTALL_PREFIX=${fmt_ROOT} -DCMAKE_CXX_STANDARD=17 -DBUILD_SHARED_LIBS=ON
cmake3 --build build --target install -- -j8

export IP6_DD4HEP_HOME=${EICTOPDIR}/detectors/ip6
git clone https://github.com/eic/ip6.git ${IP6_DD4HEP_HOME}
cmake3 -S ${IP6_DD4HEP_HOME} -B ${IP6_DD4HEP_HOME}/build -DCMAKE_INSTALL_PREFIX=${IP6_DD4HEP_HOME} -DCMAKE_CXX_STANDARD=17 -DUSE_DDG4=OFF
cmake3 --build ${IP6_DD4HEP_HOME}/build --target install -- -j8

export EIC_DD4HEP_HOME=${EICTOPDIR}/detectors/epic
git clone https://github.com/eic/epic.git ${EIC_DD4HEP_HOME}
ln -s ${IP6_DD4HEP_HOME}/ip6 ${EIC_DD4HEP_HOME}/ip6
ln -s ${IP6_DD4HEP_HOME}/ip6 ${EIC_DD4HEP_HOME}/share/epic/ip6
cmake3 -S ${EIC_DD4HEP_HOME} -B ${EIC_DD4HEP_HOME}/build -DCMAKE_INSTALL_PREFIX=${EIC_DD4HEP_HOME} -DCMAKE_CXX_STANDARD=17 -DUSE_DDG4=OFF
cmake3 --build ${EIC_DD4HEP_HOME}/build --target install -- -j8
~~~

### Capture environment
The jana_edm4hep plugin requires all of the above packages which means 
they must be set up in your environment. You will also need these in
your environment whenever you rebuild or run the code so it is a good
time to capture all of this into a file. Create a file in the EICTOPDIR
directory called _custom_environment.sh_ with the following contents:

_n.b. customize as needed if using any packages not built in the EICTOPDIR_
~~~
# scl enable devtoolset-9 rh-python38 bash

export EICTOPDIR=/path/to/my/EICTOP

source ${EICTOPDIR}/python/virtual_environments/venv/bin/activate
source ${EICTOPDIR}/root/root-6.26.04/bin/thisroot.sh

export BOOST_VERSION=boost-1.79.0
export JANA_VERSION=v2.0.7
export SPDLOG_VERSION=v1.10.0
export PODIO_VERSION=v00-14-03
export EDM4HEP_VERSION=v00-06
export EICD_VERSION=v2.0.0
export DD4HEP_VERSION=v01-20-02
export EIGEN_VERSION=3.4.0
export ACTS_VERSION=v19.4.0
export FMT_VERSION=9.0.0

export Boost_ROOT=${EICTOPDIR}/BOOST/${BOOST_VERSION}/installed
source ${EICTOPDIR}/JANA/${JANA_VERSION}/bin/jana-this.sh
export spdlog_ROOT=${EICTOPDIR}/spdlog/${SPDLOG_VERSION}/install
export PODIO_HOME=${EICTOPDIR}/PODIO/${PODIO_VERSION}
export PODIO=${PODIO_HOME}/install
source ${PODIO_HOME}/env.sh
export podio_ROOT=${PODIO}
export EDM4HEP=${EICTOPDIR}/EDM4hep/${EDM4HEP_VERSION}/install
export EDM4HEP_ROOT=${EICTOPDIR}/EDM4hep/${EDM4HEP_VERSION}/install
export LD_LIBRARY_PATH=${EDM4HEP_ROOT}/lib64:${LD_LIBRARY_PATH}
export EICD_ROOT=${EICTOPDIR}/eicd/${EICD_VERSION}/install
export LD_LIBRARY_PATH=${eicd_ROOT}/lib:${LD_LIBRARY_PATH}
source ${EICTOPDIR}/DD4hep/${DD4HEP_VERSION}/install/bin/thisdd4hep.sh
export Eigen3_ROOT=${EICTOPDIR}/EIGEN/${EIGEN_VERSION}
source ${EICTOPDIR}/ACTS/${ACTS_VERSION}/install/bin/this_acts.sh
export fmt_ROOT=${EICTOPDIR}/detectors/fmt/${FMT_VERSION}/install
export LD_LIBRARY_PATH=${fmt_ROOT}/lib64:${fmt_ROOT}/lib:${LD_LIBRARY_PATH}
source ${EICTOPDIR}/detectors/ip6/setup.sh
source ${EICTOPDIR}/detectors/epic/setup.sh

export JANA_PLUGIN_PATH=${EICTOPDIR}/EICrecon/plugins
~~~

If you are using an IDE (e.g. CLion) then the easiest way to do ensure
this environment is used is to use a wrapper script for cmake that sources
the above file each time cmake is run. Such a wrapper is included in the
_tools_ directory which automatically looks for and sources the file
_custom_environment.sh_ if it is found. To make CLion use this, go to
_Preferences->Build, Execution, Deployment->Toolchains_ and at the top
next to _CMake_ put the full path to the wrapper script. It should be
something like:
~~~
/path/to/my/EICrecon/tools/cmake_wrapper.sh
~~~

### EICrecon
The EICrecon repository is where the reconstruction code will be kept.
There is a top-level CMakeLists.txt file here that can sort-of build
everything. Well, OK. The BarrelEMCal plugin requires a header from
the jana_edm4hep and tries to get it from its JANA install location.
Thus, it is only available after jana_edm4hep has been installed. So
when you build _EICrecon_, you should do each part individually
as shown below. The system needs to be updated to be less dependent on
the install locations of the plugins, but that will wait until an
official build system is established.
~~~
git clone https://github.com/eic/EICrecon ${EICTOPDIR}/EICrecon

cd ${EICTOPDIR}/EICrecon
cmake3 -S . -B build -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Debug
cmake3 --build build --target install -- -j8
~~~

### Testing
Check that each of the plugins at least load correctly without crashing
by running them without arguments:

~~~
jana -PPLUGINS=podio,dd4hep
~~~

## EDPM installation


**edpm** stands for **E**IC **d**evelopment  **p**acket ~~**m**anager~~ helper

**The goal** of edpm is to provide esier experience of building EIC simulation and reconstruction 
framework and supporting packages on a user machine with development reasons. It is a user friendly CLI 
alternative to build instructions and build scritps that compile EIC chain of packages with right flags. 
Something that users can simply do `pip install edpm; edpm install package`.

Very minimal start: 

Install edpm itself:

```sh
python3 -m pip install --upgrade --user ejpm
```

Install eicrecon

```sh
# 1. System prerequesties
edpm req ubuntu22           # get list of required OS packets. Use `centos8` on RHEL8  
sudo apt install ...        # install watever 'edpm req' tells you

# 2. Where to install
edpm --top-dir=<where-to>   # Directory where packets will be installed

# 3. Install
edpm install eicrecon       # install 'Geant 4 EIC' and dependencies (like vgm, hepmc)

# 4.  Source environment
source ~/.local/share/edpm/env.sh  # Use *.csh file for tcsh

# Futher help. EDPM has self descriptive help just run
edpm       # current status and installed packaged
edpm --help
```

You have ROOT and Geant4 and don’t want EJPM to build them?(Use your installations of ROOT and Geant4)

```sh
# Before running 'ejpm install g4e'
edpm set root `$ROOTSYS`    # Path to ROOT installation
edpm set geant4 <path>       # Path to Geant4 installation
```

**How to switch of branch of some packet or build threads?**

```sh
# edpm has per packet build config and global config
# all packets have 'branch' config pointing to current tag/branch
edpm config jana2 branch=master   # switch jana2 to install jana2-master branch
edpm config jana2                 # see configs for jana2
edpm config global                # see global configs, such as build threads
```

(!)EDPM is not a real packet manager and don't track dependency chain. In 
current EDPM version if you re-insstall some packet to a different version, 
you have to reinstall dependent packages yourself. to reinstall some package 
(e.g. after edpm update or config change)

```
edpm rm package && edpm install package    # reinstall package

# sometimes on wants to just rebuild a package, without redownloading 
edpm install --forca package   # rebuild package even if it exists
```

**Control environment**

Packets in edpm are connected through environment, LD_LIBRARY_PATH etc. 
Every time configuration is changed (something installed or deleted) or 
***edpm env*** command is called, edpm creates 
2 environment files with the current environment: 

```bash
~/.local/share/edpm/env.sh    # bash
~/.local/share/edpm/env.csh    # bash
```

Examples: 

1. Dynamically source output of ```edpm env``` command (recommended)
    
    ```bash        
    source <(edpm env)                # works on bash
    ```
2. Save output of ```edpm env``` command to a file (can be useful)
    
    ```bash
     edpm env sh  > your-file.sh       # get environment for bash or compatible shells
     edpm env csh > your-file.csh      # get environment for CSH/TCSH
    ```

**Use CLion and others IDE with EDPM**

EDPM is designed to be used with IDEs. In order to use with VSCode or IntelliJ types of IDE:

1. Exit IDE completely (as they use a single produce even when several instances are open)
2. Source EDPM environment
3. Run `clion` or `code` from the terminal where you sources the environment

For more complex cases such as remote or docker development edpm can generate cmake
flags. See `edpm env --help`

More documentation and source: 

[https://github.com/eic/edpm](https://github.com/eic/edpm)

## plugins CMake API

There is a copy/paste CMake file that should automatically create plugin out of sources. 

- plugin name is taken from a directory name
- there should be <plugin name>.cc file with `void InitPlugin(JApplication *app)` function


### Create a plugin:

E.g. if you want to create a plugin named `my_plugin`

- Create a directory `my_plugin`
- Create a file `my_plugin.cc` which will have `InitPlugin` function
- Create `CMakeLists.txt` with the content below 
- Add all others files (cmake GLOB is used)

### Recommended cmake:

Recommended CMake for a plugin:

```cmake
cmake_minimum_required(VERSION 3.16)

# Automatically set plugin name the same as the direcotry name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# Find dependencies
find_package(JANA REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories
# (same as target_include_directories but for both plugin and library)
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${JANA_INCLUDE_DIR} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

# Add libraries
# (same as target_include_directories but for both plugin and library)
plugin_link_libraries(${PLUGIN_NAME} ${JANA_LIB})
```

### CMake macros:

There are `plugin_...` macros that are slim wrappers trying to minimize an amount of boilerplate
code of each plugin cmake scripts. Macros mimic CMake functions like `target_link_libraries` => `plugin_link_libraries`.
 

#### plugin_add

```cmake
# Function creates ${PLUGIN_NAME}_plugin target
# Sets default includes, libraries and installation paths
plugin_add(my_plugin)
```

It is possible to also automatically crate a static library from a plugin 
sources in addition to the plugin itself. Adding `WITH_STATIC_LIBRARY` to 
`plugin_add`. All other `plugin_xxx` functions will know about the second target then.

```cmake
# Now function will create 2 targets and ${PLUGIN_NAME}_plugin ${PLUGIN_NAME}_library
# one can add WITH_STATIC_LIBRARY flag to also create a static library with plugin sources
plugin_add(my_plugin WITH_STATIC_LIBRARY)
```

If `WITH_STATIC_LIBRARY` flag is given, all `plugin_...` macros will work on both targets: 
a plugin and a static library.

#### plugin_glob_all

The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
Then correctly sets sources for ${plugin_name}_plugin and ${plugin_name}_library
targets (if library is enabled). 
Adds headers to the correct installation directory

```cmake
plugin_glob_all(my_plugin)
```

#### plugin_sources

Same as target_sources both for library (if enabled) and a plugin.
If library creation is enabled, the function automatically removes 
`<plugin-name>.cc` file from library sources

```cmake
plugin_sources(my_plugin File1.cc File2.cc)
```

### plugin_include_directories

Runs target_include_directories for both a plugin and a library (if enabled)

```cmake
#example 
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC  ${ROOT_INCLUDE_DIRS})
```

### plugin_link_libraries
Runs target_link_libraries for both a plugin and a library (if enabled)

```cmake
# example
plugin_link_libraries(${PLUGIN_NAME} ${JANA_LIB})
```


## Logging

EICrecon uses [spdlog](https://github.com/gabime/spdlog) library as a logging backbone. **spdlog** utilizes
[fmt](https://github.com/fmtlib/fmt) - a string formatting and output library which uses python alike string formatting 
([is C++20 standard](https://en.cppreference.com/w/cpp/utility/format)) and is very performant 
(in many cases [is faster than printf and std streams](https://github.com/fmtlib/fmt#speed-tests))


### Basic usage

EICRecon has a log service that centralizes default logger configuration and helps spawn named loggers. 
Each unit - a plugin, factory, class, etc. can spawn its own named logger and use it: 

[FULL WORKING EXAMPLE](https://github.com/eic/EICrecon/blob/main/src/examples/log_example/log_example.cc)

```c++
#include <services/log/Log_service.h>
class ExampleProcessor: public JEventProcessor {
private:
    std::shared_ptr<spdlog::logger> m_log;

public:
    /// Once in app lifetime function call
    void Init() override {        

        auto app = GetApplication();

        // The service centralizes the use of spdlog and properly spawning logger
        auto log_service = app->GetService<Log_service>();

        // Loggers are spawned by name.        
        m_log = log_service->logger("ExampleProcessor");
        
        // log things!
        m_log->info("Hello world! {}", 42);
    }
    
    /// The function is executed every event
    void Process(const std::shared_ptr<const JEvent>& event) override {
        // Will print something if 'trace' log level is set (see below)
        m_log->trace("Processing event #{}", event->GetEventNumber());
        // ...
    }
```

Thanks to fmt, logger has rich text formatting. More examples and full 
specification is in [fmt documentation](https://github.com/fmtlib/fmt): 

```c++
m_log->info("Welcome to spdlog!");
// [info] Welcome to spdlog!

m_log->error("Some error message with arg: {}", 1);
// [error] Some error message with arg: 1

m_log->warn("Easy padding in numbers like {:08d}", 12);
// [warning] Easy padding in numbers like 00000012

m_log->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
// [critical] Support for int: 42;  hex: 2a;  oct: 52; bin: 101010

m_log->info("Support for floats number of digits {:03.2f}", 1.23456);
// [info] Support for floats number of digits 1.23

m_log->info("Positional args are {1} {0}...", "too", "supported");
// [info] Positional args are supported too...

m_log->info("{:>30}", "right aligned");
// [info]                  right aligned

m_log->info("Table of values:");
m_log->info("{:<5} {:<7} {:>10}", "N", "val1", "val2");
m_log->info("{:=^30}", "centered");  // use '=' as a fill char
m_log->info("{:<5} {:<7.2f} {:>10}", 23, 3.1415, 3.1415);
m_log->info("{:<5} {:<7.2f} {:>10}", 24, 2.7182, 2.7182);
m_log->info("{:<5} {:<7.2f} {:>10}", 25, 1.6180, 1.6180);
// [info] Table of values:
// [info] N     val1          val2
// [info] ===========centered===========
// [info] 23    3.14        3.1415
// [info] 24    2.72        2.7182
// [info] 25    1.62         1.618

// Compile time log levels
// define SPDLOG_ACTIVE_LEVEL to desired level e.g.:
//    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
SPDLOG_TRACE("Some trace message with param {}", 42);
SPDLOG_DEBUG("Some debug message");
```

In order to wire your logger level with a jana-parameter to change log level without recompilation, use: 

```c++
// includes: 
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

// ... all from previous example
void Init() override {
   // ...
   
   // This is a default level  
   std::string log_level_str = "info";
   
   // Ask service locator for parameter manager. We want to get this plugin parameters.
   auto pm = app->GetJParameterManager();
   
   // Define parameter
   pm->SetDefaultParameter("log_example:log-level", log_level_str, "log_level: trace, debug, info, warn, err, critical, off");
   
   // At this point log_level_str is initialized with user provided flag value or the default value
   
   // convert input std::string to spdlog::level::level_enum and set logger level
   m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
}
```

**How log levels should be used?**

- **trace**    - something very verbose like each hit parameter
- **debug**    - all information that is relevant for an expert to debug but should not be present in production
- **info**     - something that will always (almost) get into the global log
- **warning**  - something that needs attention but results are likely usable
- **error**    - something bad making results probably unusable
- **critical** - imminent software failure and termination

Sometimes one needs to know current log level to calculate debugging values. Use **<=** operator to check level.
It works because enum values are: trace=0, debug=1, ... critical= 5. So:

```c++
  // In general one doesn't need to manually check log debug level
  m_log->debug("Will be printed if level is 'debug' or 'trace' ");

  // But sometimes one needs to know current log level to calculate debugging values
  // Use  <= operator to check level.
  // m_log->level() <= spdlog::level::debug  means 'debug' or 'trace'
  // m_log->level() <= spdlog::level::info   means 'info', 'debug' or 'trace' levels
  if(m_log->level() <= spdlog::level::debug) {
      int x = event->GetRunNumber()*1000000 + event->GetEventNumber()/2;
      m_log->debug("Calculated debug value #{}", x);
  }
```

### Logging hints


#### Streamable objects

Sometimes you need to print objects with custom overloaded ostr stream operator `<<`
i.e. objects that knows how to print self when used with something like  ```cout<<object```
Such objects are used sometimes, e.g. lib eigen matrices might be printed that way. 
To enable printing of such objects include the next header

```c++
// Include this to print/format streamable objects.
// You don't need this header most of the time (until you see a compilation error)
#include <spdlog/fmt/ostr.h>
```

#### Default logger

spdlog has a default logger and global functions like `spdlog::info`, `spdlog::warn` etc.  

```c++
spdlog::info("Hello world from default logger! You can do this, but please don't");
```

It is possible to use a default logger to print something out, but it is NOT RECOMMENDED to be used instead
of a named loggers. Default logger is used to highlight something that relates to whole application execution. 
Otherwise, use named logger. 


#### Shared names

By default, spdlog fails if a logger with such name exists (but one can get existing logger
from registry). EICrecon Logger service simplifies and automates it with a single function `logger(name)`. 
This allows to use the same logger with the same name from different units if the context is the same. 
Imagine you want to highlight that this message belongs to "tracking" you can do: 

```c++

// One class
m_tracking_log = app->GetService<Log_service>()->logger("tracking");

// Another class
m_tracking_log = app->GetService<Log_service>()->logger("tracking");
```

You can mix named loggers depending on context

```c++

// Some class INIT
m_this_log = app->GetService<Log_service>()->logger("ExampleFactoryName");
m_tracking_log = app->GetService<Log_service>()->logger("tracking");

// Some class event PROCESSING
m_this_log->trace("Something related to this class/factory/plugin");
m_tracking_log->info("Something relating to tracking in general");
```

#### String format

It is recommended to use fmt string formatting both from performance and safety points. But it is also very convenient!
fmt can be accessed through spdlog headers: 

```c++
#include <spdlog/fmt/fmt.h>

// code
std::string hello = fmt::format("Hello world {}", 42);
```

#### CMake

spdlog is included by default in every plugin if `plugin_add` macro is used: 

```cmake
plugin_add(${PLUGIN_NAME})  # <= spdlog will be included by default
```

eicrecon application also includes Log_service by default. So it should not appear on `-Pplugins` list. 


### Logging links

- [spdlog](https://github.com/gabime/spdlog)
- [fmt](https://github.com/fmtlib/fmt)
- [EICrecon logging examples](https://github.com/eic/EICrecon/blob/main/src/examples/log_example/log_example.cc)
- [EICrecon factory with log example](https://github.com/eic/EICrecon/blob/main/src/algorithms/tracking/TrackerHitReconstruction_factory.cc)