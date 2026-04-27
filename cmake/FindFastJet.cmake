# * Locate FastJet library Defines:
#
# FASTJET_FOUND FASTJET_INCLUDE_DIR FASTJET_INCLUDE_DIRS (not cached)
# FASTJET_LIBRARY FASTJET_LIBRARIES (not cached) FASTJET_LIBRARY_DIRS (not
# cached)

find_library(
  FASTJET_LIBRARY
  NAMES fastjet
  HINTS $ENV{FASTJET_ROOT}/lib ${FASTJET_ROOT_DIR}/lib)

# Resolve symlinks on the library to derive the real package prefix.
set(_fastjet_include_hints)
if(FASTJET_LIBRARY)
  # cmake-lint: disable=E1126
  file(REAL_PATH "${FASTJET_LIBRARY}" _fastjet_real_lib)
  get_filename_component(_fastjet_lib_dir "${_fastjet_real_lib}" DIRECTORY)
  get_filename_component(_fastjet_prefix "${_fastjet_lib_dir}" DIRECTORY)
  list(APPEND _fastjet_include_hints "${_fastjet_prefix}/include")
  unset(_fastjet_real_lib)
  unset(_fastjet_lib_dir)
  unset(_fastjet_prefix)
endif()

find_path(FASTJET_INCLUDE_DIR fastjet/version.hh
          HINTS ${_fastjet_include_hints} $ENV{FASTJET_ROOT}/include
                ${FASTJET_ROOT_DIR}/include
          NO_DEFAULT_PATH)

unset(_fastjet_include_hints)

# handle the QUIETLY and REQUIRED arguments and set FASTJET_FOUND to TRUE if all
# listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FastJet DEFAULT_MSG FASTJET_INCLUDE_DIR
                                  FASTJET_LIBRARY)

mark_as_advanced(FASTJET_FOUND FASTJET_INCLUDE_DIR FASTJET_LIBRARY)

set(FASTJET_INCLUDE_DIRS ${FASTJET_INCLUDE_DIR})
set(FASTJET_LIBRARIES ${FASTJET_LIBRARY})
get_filename_component(FASTJET_LIBRARY_DIRS ${FASTJET_LIBRARY} PATH)
