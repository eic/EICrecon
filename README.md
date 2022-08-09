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
from source. Be aware that this is the longest step in the whole process
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
export JANA_VERSION=v2.0.5
export JANA_HOME=${EICTOPDIR}/JANA/${JANA_VERSION}
git clone https://github.com/JeffersonLab/JANA2 -b ${JANA_VERSION} ${JANA_HOME}
cd ${JANA_HOME}
cmake3 -S . -B build -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=${JANA_HOME} -DUSE_ROOT=1 # -DUSE_ZEROMQ=1 -DUSE_PYTHON=1
cmake3 --build build --target install -- -j8
source ${JANA_HOME}/bin/jana-this.sh                # Set environment to use this
~~~

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
export EDM4HEP_VERSION=v00-05
export EDM4HEP_HOME=${EICTOPDIR}/EDM4hep/${EDM4HEP_VERSION}
export EDM4HEP=${EDM4HEP_HOME}/install 
export EDM4HEP_ROOT=${EDM4HEP}
git clone https://github.com/key4hep/EDM4hep -b ${EDM4HEP_VERSION} ${EDM4HEP_HOME}
cd ${EDM4HEP_HOME}
cmake3 -S . -B build -DCMAKE_INSTALL_PREFIX=${EDM4HEP} -DCMAKE_CXX_STANDARD=17 -DUSE_EXTERNAL_CATCH2=OFF
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
ln -s ${IP6_DD4HEP_HOME} ${EIC_DD4HEP_HOME}/share/epic/ip6
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
export JANA_VERSION=v2.0.5
export PODIO_VERSION=v00-14-03
export EDM4HEP_VERSION=v00-05
export DD4HEP_VERSION=v01-20-02
export EIGEN_VERSION=3.4.0
export ACTS_VERSION=v19.4.0
export FMT_VERSION=9.0.0

export Boost_ROOT=${EICTOPDIR}/BOOST/${BOOST_VERSION}/installed
source ${EICTOPDIR}/JANA/${JANA_VERSION}/bin/jana-this.sh
export PODIO_HOME=${EICTOPDIR}/PODIO/${PODIO_VERSION}
export PODIO=${PODIO_HOME}/install
export PODIO_ROOT=${PODIO}
source ${PODIO_HOME}/env.sh
export podio_ROOT=${PODIO}
export EDM4HEP=${EICTOPDIR}/EDM4hep/${EDM4HEP_VERSION}/install
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
find_package(ROOT REQUIRED)

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