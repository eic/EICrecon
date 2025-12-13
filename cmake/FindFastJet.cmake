# * Locate FastJet library Defines:
#
# FASTJET_FOUND FASTJET_INCLUDE_DIR FASTJET_INCLUDE_DIRS (not cached)
# FASTJET_LIBRARY FASTJET_LIBRARIES (not cached) FASTJET_LIBRARY_DIRS (not
# cached)

# Allow user to specify FASTJET_ROOT as a hint
if(NOT DEFINED FASTJET_ROOT)
  set(FASTJET_ROOT "")
endif()

# Allow user to specify FASTJET_ROOT_DIR as a hint
if(NOT DEFINED FASTJET_ROOT_DIR)
  set(FASTJET_ROOT_DIR "")
endif()

find_path(FASTJET_INCLUDE_DIR fastjet/version.hh
          HINTS $ENV{FASTJET_ROOT}/include ${FASTJET_ROOT_DIR}/include)

find_library(
  FASTJET_LIBRARY
  NAMES fastjet
  HINTS $ENV{FASTJET_ROOT}/lib ${FASTJET_ROOT_DIR}/lib)

# handle the QUIETLY and REQUIRED arguments and set FASTJET_FOUND to TRUE if all
# listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FastJet DEFAULT_MSG FASTJET_INCLUDE_DIR
                                  FASTJET_LIBRARY)

mark_as_advanced(FASTJET_FOUND FASTJET_INCLUDE_DIR FASTJET_LIBRARY)

set(FASTJET_INCLUDE_DIRS ${FASTJET_INCLUDE_DIR} CACHE STRING "FastJet include directories")
set(FASTJET_LIBRARIES ${FASTJET_LIBRARY} CACHE STRING "FastJet libraries")
get_filename_component(FASTJET_LIBRARY_DIRS ${FASTJET_LIBRARY} PATH)
mark_as_advanced(FASTJET_INCLUDE_DIRS FASTJET_LIBRARIES)