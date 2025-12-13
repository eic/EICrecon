# * Locate FastJetContrib library Defines:
#
# FJCONTRIB_FOUND FJCONTRIB_INCLUDE_DIR FJCONTRIB_INCLUDE_DIRS (not cached)
# FJCONTRIB_LIBRARY FJCONTRIB_LIBRARIES (not cached) FJCONTRIB_LIBRARY_DIRS (not
# cached)

# Allow user to specify FASTJET_ROOT as a hint
if(NOT DEFINED FASTJET_ROOT)
  set(FASTJET_ROOT "")
endif()

# Allow user to specify FASTJET_ROOT_DIR as a hint
if(NOT DEFINED FASTJET_ROOT_DIR)
  set(FASTJET_ROOT_DIR "")
endif()

find_library(
  FJCONTRIB_LIBRARY
  NAMES fastjetcontribfragile
  HINTS $ENV{FASTJET_ROOT}/lib ${FASTJET_ROOT_DIR}/lib)

# Resolve symlinks on the library to derive the real package prefix.
set(_fjcontrib_include_hints)
if(FJCONTRIB_LIBRARY)
  # cmake-lint: disable=E1126
  file(REAL_PATH "${FJCONTRIB_LIBRARY}" _fjcontrib_real_lib)
  get_filename_component(_fjcontrib_lib_dir "${_fjcontrib_real_lib}" DIRECTORY)
  get_filename_component(_fjcontrib_prefix "${_fjcontrib_lib_dir}" DIRECTORY)
  list(APPEND _fjcontrib_include_hints "${_fjcontrib_prefix}/include")
  unset(_fjcontrib_real_lib)
  unset(_fjcontrib_lib_dir)
  unset(_fjcontrib_prefix)
endif()

find_path(
  FJCONTRIB_INCLUDE_DIR fastjet/contrib/Centauro.hh
  HINTS ${_fjcontrib_include_hints} $ENV{FASTJET_ROOT}/include
        ${FASTJET_ROOT_DIR}/include
  NO_DEFAULT_PATH)

unset(_fjcontrib_include_hints)

# handle the QUIETLY and REQUIRED arguments and set FJCONTRIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FastJetContrib DEFAULT_MSG
                                  FJCONTRIB_INCLUDE_DIR FJCONTRIB_LIBRARY)

mark_as_advanced(FJCONTRIB_FOUND FJCONTRIB_INCLUDE_DIR FJCONTRIB_LIBRARY)

set(FJCONTRIB_INCLUDE_DIRS ${FJCONTRIB_INCLUDE_DIR} CACHE STRING "FastJet Contrib include directories")
set(FJCONTRIB_LIBRARIES ${FJCONTRIB_LIBRARY} CACHE STRING "FastJet Contrib libraries")
get_filename_component(FJCONTRIB_LIBRARY_DIRS ${FJCONTRIB_LIBRARY} PATH)
mark_as_advanced(FJCONTRIB_INCLUDE_DIRS FJCONTRIB_LIBRARIES)