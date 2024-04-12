# * Locate FastJet Tools library Defines:
#
# FJTOOLS_FOUND FJTOOLS_INCLUDE_DIR FJTOOLS_INCLUDE_DIRS (not cached)
# FJTOOLS_LIBRARY FJTOOLS_LIBRARIES (not cached) FJTOOLS_LIBRARY_DIRS (not
# cached)

find_path(FJTOOLS_INCLUDE_DIR fastjet/tools/BackgroundEstimatorBase.hh
          HINTS $ENV{FASTJET_ROOT}/include ${FASTJET_ROOT_DIR}/include)

find_library(
  FJTOOLS_LIBRARY
  NAMES fastjettools
  HINTS $ENV{FASTJET_ROOT}/lib ${FASTJET_ROOT_DIR}/lib)

# handle the QUIETLY and REQUIRED arguments and set FJTOOLS_FOUND to TRUE if all
# listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FastJetTools DEFAULT_MSG FJTOOLS_INCLUDE_DIR
                                  FJTOOLS_LIBRARY)

mark_as_advanced(FJTOOLS_FOUND FJTOOLS_INCLUDE_DIR FJTOOLS_LIBRARY)

set(FJTOOLS_INCLUDE_DIRS ${FJTOOLS_INCLUDE_DIR})
set(FJTOOLS_LIBRARIES ${FJTOOLS_LIBRARY})
get_filename_component(FJTOOLS_LIBRARY_DIRS ${FJTOOLS_LIBRARY} PATH)
