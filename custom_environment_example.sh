#
# This is an example of a custom environment setup file. This is
# useful if you want to use the tools/cmake_wrapper.sh script
# in place of "cmake" in your IDE to ensure the proper environment
# is used when running cmake.
#
# This is not required if you set your environment through other
# means (e.g. login script).
#
# This is just an example. You should rename this to
# "custom_environment.sh" and edit it to your specific system.

# Get directrory this script resides in
#SCRIPT_DIR=`dirname -- "$0"`
SCRIPT_DIR=`readlink -f $(dirname -- "$0")`

# Setup environment (customize for your system)
source /Users/davidl/work/2022.07.06.JANA2_DD4hep/venv/bin/activate
source /Users/davidl/HallD/builds/root/root-6.26.04/bin/thisroot.sh
source /Users/davidl/work/2022.07.06.JANA2_DD4hep/JANAv2.0.5/bin/jana-this.sh
export PODIO=${SCRIPT_DIR}/I_O/PODIO_v00-14-03/install
[ -f ${PODIO}/../env.sh ] && source ${PODIO}/../env.sh

