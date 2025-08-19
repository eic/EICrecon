#!/bin/bash
#
# Cut'n'paste from Chris's environ.sh file
#

# drich-dev path
if [ -z "${BASH_SOURCE[0]}" ]; then
  export IRT_DEV=$(dirname $(realpath $0))
else
  export IRT_DEV=$(dirname $(realpath ${BASH_SOURCE[0]}))
fi

# obtain number of CPUs
# - set it manually, if you prefer, or if auto detection fails
export BUILD_NPROC=$([ $(uname) = 'Darwin' ] && sysctl -n hw.ncpu || nproc)
if [ "$BUILD_NPROC" = "" ]; then export BUILD_NPROC=1; fi
echo "detected $BUILD_NPROC cpus"

# local installation prefix
export EIC_SHELL_PREFIX=$IRT_DEV/prefix

# variable needed for `dawn`
export LOCAL_DATA_PATH=$IRT_DEV

# source common upstream environment (nightly jug_xl build)
source /opt/detector/epic-main/setup.sh

# source local environment (a build target from `epic`)
# - overrides upstream `$DETECTOR*` vars
# - prioritizes `$EIC_SHELL_PREFIX/lib` in `$LD_LIBRARY_PATH`
[ -f $EIC_SHELL_PREFIX/setup.sh ] && source $EIC_SHELL_PREFIX/setup.sh

# source EICrecon installation + environment patches
if [ -f $EIC_SHELL_PREFIX/bin/eicrecon-this.sh ]; then
  if [ -z "$CI" ]; then
    echo "PATCH: exclude container's EICrecon plugins from JANA_PLUGIN_PATH"
    exc="/opt/local/lib/EICrecon/plugins"
    export JANA_PLUGIN_PATH=$(echo $JANA_PLUGIN_PATH | sed "s;${exc}:;;g" | sed "s;:${exc};;g" | sed "s;${exc};;g" )
    echo "ENVIRONMENT: source EICrecon"
    source $EIC_SHELL_PREFIX/bin/eicrecon-this.sh
    echo "PATCH: source thisroot.sh removes /opt/local/bin from PATH; add it back"
    export PATH="$PATH:/opt/local/bin"
  else
    echo "On CI runner; only setting JANA_PLUGIN_PATH"
    export JANA_PLUGIN_PATH=$EIC_SHELL_PREFIX/lib/EICrecon/plugins${JANA_PLUGIN_PATH:+:${JANA_PLUGIN_PATH}}
  fi
fi

# check if we have ROOT I/O enabled for IRT
export IRT_ROOT_DICT_FOUND=0
if [ -f $EIC_SHELL_PREFIX/lib/libIRT_rdict.pcm -a -f $EIC_SHELL_PREFIX/lib/libIRT.rootmap ]; then
  export IRT_ROOT_DICT_FOUND=1
elif [ -f /opt/local/lib/libIRT_rdict.pcm -a -f /opt/local/lib/libIRT.rootmap ]; then
  export IRT_ROOT_DICT_FOUND=1
fi

# environment overrides:
# - prefer local juggler build
export JUGGLER_INSTALL_PREFIX=$EIC_SHELL_PREFIX
export JUGGLER_N_THREADS=$BUILD_NPROC
# - update prompt
export PS1="${PS1:-}"
export PS1="irt${PS1_SIGIL}>${PS1#*>}"
unset branch

# prioritize local build targets
export LD_LIBRARY_PATH=$IRT_DEV/lib:$EIC_SHELL_PREFIX/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$EIC_SHELL_PREFIX/python:$PYTHONPATH
export PATH=$EIC_SHELL_PREFIX/bin:$PATH

# use local rbenv ruby shims, if installed
export RBENV_ROOT=$IRT_DEV/.rbenv
if [ -d "$RBENV_ROOT" ]; then
  export PATH=$RBENV_ROOT/bin:$PATH
  eval "$(.rbenv/bin/rbenv init - bash)"
  export PYTHON=$(which python) # for pycall gem
fi

# additional comfort settings; add your own here
# - PATH additions
export PATH=.:$PATH                                   # ./
export PATH=$IRT_DEV/bin:$PATH                      # irt-dev/bin
[ -d "${HOME}/bin" ] && export PATH=$PATH:${HOME}/bin # ~/bin
# - shell settings and aliases
shopt -s autocd # enable autocd (`alias <dirname>='cd <dirname>'`)
alias ll='ls -lhp --color=auto'
# - open a ROOT file in a TBrowser
broot() {
  if [ $# -ne 1 ]; then
    echo "USAGE: $0 [ROOT file]"
  else
    root -l --web=off $1 -e 'new TBrowser'
  fi
}

# print environment
echo """


     ###########################################
     ###    IRT Development Environment    ###
     ###########################################

Detector:
  DETECTOR         = $DETECTOR
  DETECTOR_PATH    = $DETECTOR_PATH
  DETECTOR_CONFIG  = $DETECTOR_CONFIG
  DETECTOR_VERSION = $DETECTOR_VERSION

LD_LIBRARY_PATH:
  $(echo $LD_LIBRARY_PATH | sed 's/:/\n  /g')

Common:
  IRT_DEV             = $IRT_DEV
  BUILD_NPROC         = $BUILD_NPROC
  EIC_SHELL_PREFIX    = $EIC_SHELL_PREFIX
  JANA_PLUGIN_PATH    = $JANA_PLUGIN_PATH
  IRT_ROOT_DICT_FOUND = $IRT_ROOT_DICT_FOUND

"""
