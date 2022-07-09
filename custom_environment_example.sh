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

# Setup environment (customize for your system)
export EICTOPDIR=/Users/davidl/work/2022.07.09.EICrecon/EICTOP

source ${EICTOPDIR}/root/root-6.26.04/bin/thisroot.sh
source ${EICTOPDIR}/JANA/v2.0.5/bin/jana-this.sh
export PODIO=${EICTOPDIR}/PODIO/v00-14-03/install
source ${PODIO}/../env.sh
export EDM4HEP=${EICTOPDIR}/EDM4HEP/v00-05/install
source ${EDM4HEP}/../init.sh

