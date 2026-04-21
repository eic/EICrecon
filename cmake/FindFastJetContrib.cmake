# * Locate FastJetContrib library Defines:
#
# FJCONTRIB_FOUND FJCONTRIB_INCLUDE_DIR FJCONTRIB_INCLUDE_DIRS (not cached)
# FJCONTRIB_LIBRARY FJCONTRIB_LIBRARIES (not cached) FJCONTRIB_LIBRARY_DIRS (not
# cached)

find_library(
  FJCONTRIB_LIBRARY
  NAMES fastjetcontribfragile
  HINTS $ENV{FASTJET_ROOT}/lib ${FASTJET_ROOT_DIR}/lib)

# Resolve symlinks on the library to derive the real package prefix.
if(FJCONTRIB_LIBRARY)
  file(REAL_PATH "${FJCONTRIB_LIBRARY}" _fjcontrib_real_lib)
  get_filename_component(_fjcontrib_lib_dir "${_fjcontrib_real_lib}" DIRECTORY)
  get_filename_component(_fjcontrib_prefix "${_fjcontrib_lib_dir}" DIRECTORY)
else()
  set(_fjcontrib_prefix "")
endif()

find_path(
  FJCONTRIB_INCLUDE_DIR fastjet/contrib/Centauro.hh
  HINTS ${_fjcontrib_prefix}/include $ENV{FASTJET_ROOT}/include
        ${FASTJET_ROOT_DIR}/include
  NO_CMAKE_ENVIRONMENT_PATH)

unset(_fjcontrib_real_lib)
unset(_fjcontrib_lib_dir)
unset(_fjcontrib_prefix)

# handle the QUIETLY and REQUIRED arguments and set FJCONTRIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FastJetContrib DEFAULT_MSG
                                  FJCONTRIB_INCLUDE_DIR FJCONTRIB_LIBRARY)

mark_as_advanced(FJCONTRIB_FOUND FJCONTRIB_INCLUDE_DIR FJCONTRIB_LIBRARY)

set(FJCONTRIB_INCLUDE_DIRS ${FJCONTRIB_INCLUDE_DIR})
set(FJCONTRIB_LIBRARIES ${FJCONTRIB_LIBRARY})
get_filename_component(FJCONTRIB_LIBRARY_DIRS ${FJCONTRIB_LIBRARY} PATH)
