# * Locate FastJet Tools library Defines:
#
# FJTOOLS_FOUND FJTOOLS_INCLUDE_DIR FJTOOLS_INCLUDE_DIRS (not cached)
# FJTOOLS_LIBRARY FJTOOLS_LIBRARIES (not cached) FJTOOLS_LIBRARY_DIRS (not
# cached)

find_library(
  FJTOOLS_LIBRARY
  NAMES fastjettools
  HINTS $ENV{FASTJET_ROOT}/lib ${FASTJET_ROOT_DIR}/lib)

# Resolve symlinks on the library to derive the real package prefix.
set(_fjtools_include_hints)
if(FJTOOLS_LIBRARY)
  # cmake-lint: disable=E1126
  file(REAL_PATH "${FJTOOLS_LIBRARY}" _fjtools_real_lib)
  get_filename_component(_fjtools_lib_dir "${_fjtools_real_lib}" DIRECTORY)
  get_filename_component(_fjtools_prefix "${_fjtools_lib_dir}" DIRECTORY)
  list(APPEND _fjtools_include_hints "${_fjtools_prefix}/include")
  unset(_fjtools_real_lib)
  unset(_fjtools_lib_dir)
  unset(_fjtools_prefix)
endif()

find_path(
  FJTOOLS_INCLUDE_DIR fastjet/tools/BackgroundEstimatorBase.hh
  HINTS ${_fjtools_include_hints} $ENV{FASTJET_ROOT}/include
        ${FASTJET_ROOT_DIR}/include)

unset(_fjtools_include_hints)

# handle the QUIETLY and REQUIRED arguments and set FJTOOLS_FOUND to TRUE if all
# listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FastJetTools DEFAULT_MSG FJTOOLS_INCLUDE_DIR
                                  FJTOOLS_LIBRARY)

mark_as_advanced(FJTOOLS_FOUND FJTOOLS_INCLUDE_DIR FJTOOLS_LIBRARY)

set(FJTOOLS_INCLUDE_DIRS ${FJTOOLS_INCLUDE_DIR})
set(FJTOOLS_LIBRARIES ${FJTOOLS_LIBRARY})
get_filename_component(FJTOOLS_LIBRARY_DIRS ${FJTOOLS_LIBRARY} PATH)
