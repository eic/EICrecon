#!/bin/bash
#
# A cut down version of Chris's original file;
#

# set working directory
export SANDBOX=`pwd`

# obtain number of CPUs
# - set it manually, if you prefer, or if auto detection fails
export BUILD_NPROC=$([ $(uname) = 'Darwin' ] && sysctl -n hw.ncpu || nproc)
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
echo "detected $BUILD_NPROC cpus"

# local installation prefix
export EIC_SHELL_PREFIX=$SANDBOX/prefix

# source common upstream environment (nightly jug_xl build)
source /opt/detector/epic-main/setup.sh

# source EICrecon installation + environment patches
if [ -f $EIC_SHELL_PREFIX/bin/eicrecon-this.sh ]; then
  source $EIC_SHELL_PREFIX/bin/eicrecon-this.sh
fi

# update prompt
export PS1="${PS1:-}"
export PS1="+irt${PS1_SIGIL}>${PS1#*>}"

# prioritize local build targets
export PYTHONPATH=$EIC_SHELL_PREFIX/python:$PYTHONPATH
export PATH=$EIC_SHELL_PREFIX/bin:$PATH:/opt/local/bin
export LD_LIBRARY_PATH=$EIC_SHELL_PREFIX/lib:${LD_LIBRARY_PATH}
export DETECTOR_PATH=$EIC_SHELL_PREFIX/share/epic
export JANA_PLUGIN_PATH=$EIC_SHELL_PREFIX/lib/EICrecon/plugins:$JANA_PLUGIN_PATH
export ROOT_LIBRARY_PATH=$EIC_SHELL_PREFIX/lib:${ROOT_LIBRARY_PATH}

# additional comfort settings; add your own here
shopt -s autocd # enable autocd (`alias <dirname>='cd <dirname>'`)

# print environment
echo """

     ###########################################
     ###   IRT-2.0 Development Environment   ###
     ###########################################

Detector:
  DETECTOR         = $DETECTOR
  DETECTOR_PATH    = $DETECTOR_PATH
  DETECTOR_CONFIG  = $DETECTOR_CONFIG
  DETECTOR_VERSION = $DETECTOR_VERSION

LD_LIBRARY_PATH:
  $(echo $LD_LIBRARY_PATH | sed 's/:/\n  /g')

Common:
  SANDBOX             = $SANDBOX
  BUILD_NPROC         = $BUILD_NPROC
  EIC_SHELL_PREFIX    = $EIC_SHELL_PREFIX
  JANA_PLUGIN_PATH    = $JANA_PLUGIN_PATH

"""
