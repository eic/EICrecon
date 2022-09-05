#!/bin/bash
#
# This script is used as a replacement for calling cmake
# directly so that the environment can be properly set
# up. It is intended to be used by IDEs (e.g. CLion).
#
# The specifics of your environment should be setup in a
# file named "EICrecon/custom_environment.sh" (i.e. in
# the directory directly above this one). If this file
# exists, it will be sourced before running cmake.


# Get directrory this script resides in
SCRIPT_DIR=`readlink -f $(dirname -- "$0")`

if [ -f ${SCRIPT_DIR}/../../custom_environment.sh ]; then
  #echo source ${SCRIPT_DIR}/../../custom_environment.sh
  source ${SCRIPT_DIR}/../../custom_environment.sh
elif [ -f ${SCRIPT_DIR}/../custom_environment.sh ]; then
  #echo source ${SCRIPT_DIR}/../custom_environment.sh
  source ${SCRIPT_DIR}/../custom_environment.sh

fi

# Run cmake
cmake "$@"
