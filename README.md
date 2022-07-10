# EICrecon
EIC Reconstruction - JANA based

## Build Instructions
These are temporary build instructions as the build system and environment
setup system needs to be better developed. These include building all of
the dependencies. 

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
mkdir root-6.26.04.build root-6.26.04
cd root-6.26.04.build
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=../root-6.26.04 -Dbuiltin_glew=ON ../root-6.26.04.src
make -j8 install

source ${EICTOPDIR}/root/root-6.26.04/bin/thisroot.sh
~~~

### JANA
~~~
export JANA_VERSION=v2.0.5                          # Just for convenience here
export JANA_HOME=${EICTOPDIR}/JANA/${JANA_VERSION}  # Set full path to install dir
git clone https://github.com/JeffersonLab/JANA2 -b ${JANA_VERSION} ${JANA_HOME}
mkdir ${JANA_HOME}/build                            # Set build dir
cd ${JANA_HOME}/build
cmake -DUSE_ROOT=1 ../                              # (add -DUSE_ZEROMQ=1 if you have ZeroMQ available)
make -j8 install

source ${JANA_HOME}/bin/jana-this.sh                # Set environment to use this
~~~

### PODIO
~~~
export PODIO_VERSION=v00-14-03
export PODIO_HOME=${EICTOPDIR}/PODIO/${PODIO_VERSION}
git clone https://github.com/AIDASoft/podio -b v00-14-03 ${PODIO_HOME}
cd ${PODIO_HOME}
source init.sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install -DUSE_EXTERNAL_CATCH2=OFF ../
make -j8 install

export PODIO=${PODIO_HOME}/install
source ${PODIO_HOME}/env.sh
~~~

### EDM4hep
~~~
export EDM4HEP_VERSION=v00-05
export EDM4HEP_HOME=${EICTOPDIR}/EDM4hep/${EDM4HEP_VERSION}
git clone https://github.com/key4hep/EDM4hep -b ${EDM4HEP_VERSION} ${EDM4HEP_HOME}
cd ${EDM4HEP_HOME}
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install -DUSE_EXTERNAL_CATCH2=OFF ../
make -j8 install

export EDM4HEP=${EDM4HEP_HOME}/install 
~~~

### Capture environment
The jana_edm4hep plugin requires all of the above packages which means 
they must be set up in your environment. You will also need these in
your environment whenever you rebuild or run the code so it is a good
time to capture all of this into a file. Create a file in the EICTOPDIR
directory called _custom_environment.sh_ with the following contents:

_n.b. customize as needed if using any packages not built in the EICTOPDIR_
~~~
export EICTOPDIR=/path/to/my/EICTOP

source ${EICTOPDIR}/python/virtual_environments/venv/bin/activate
source ${EICTOPDIR}/root/root-6.26.04/bin/thisroot.sh
source ${EICTOPDIR}/JANA/v2.0.5/bin/jana-this.sh
export PODIO=${EICTOPDIR}/PODIO/v00-14-03/install
source ${PODIO}/../env.sh
export EDM4HEP=${EICTOPDIR}/EDM4HEP/v00-05
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
Clone this.

~~~
cd ${EICTOPDIR}
git clone https://github.com/eic/EICrecon
~~~

### jana_edm4hep
This JANA plugin can read an EDM4hep root file. It requires all of the above
packages.

~~~
mkdir ${EICTOPDIR}/EICrecon/I_O/plugins/jana_edm4hep/build
cd ${EICTOPDIR}/EICrecon/I_O/plugins/jana_edm4hep/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j8 install
~~~
