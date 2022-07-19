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
export EDM4HEP_ROOT=${EDM4HEP}
source ${EICTOPDIR}/DD4hep/${DD4HEP_VERSION}/install/bin/thisdd4hep.sh
export Eigen3_ROOT=${EICTOPDIR}/EIGEN/${EIGEN_VERSION}
source ${EICTOPDIR}/ACTS/${ACTS_VERSION}/install/bin/this_acts.sh
export fmt_ROOT=${EICTOPDIR}/detectors/fmt/${FMT_VERSION}/install
export LD_LIBRARY_PATH=${fmt_ROOT}/lib64:${fmt_ROOT}/lib:${LD_LIBRARY_PATH}
export IP6_DD4HEP_HOME=${EICTOPDIR}/detectors/ip6
export EIC_DD4HEP_HOME=${EICTOPDIR}/detectors/ecce
export EIC_DD4HEP_XML=${EIC_DD4HEP_HOME}/ecce.xml