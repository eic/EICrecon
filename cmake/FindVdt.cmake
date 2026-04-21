# Wrapper around ROOT's FindVdt.cmake that resolves Spack view symlinks so that
# the package-specific include path (under /opt/software/…/<hash>/) is used
# instead of the merged view path (/opt/local/include).
#
# ROOT's FindVdt.cmake guards both find_library() and find_path() with "if(NOT
# VDT_LIBRARY)" / "if(NOT VDT_INCLUDE_DIR)", so pre-setting these cache
# variables here causes it to skip its own search and use our values.

# Step 1: find the library (may resolve to a spack-view symlink).
if(NOT VDT_LIBRARY)
  find_library(VDT_LIBRARY NAMES vdt)
endif()

# Step 2: resolve the library symlink to derive the real package prefix and
# pre-set VDT_INCLUDE_DIR before ROOT's finder runs its find_path().
if(VDT_LIBRARY AND NOT VDT_INCLUDE_DIR)
  file(REAL_PATH "${VDT_LIBRARY}" _vdt_real_lib)
  get_filename_component(_vdt_lib_dir "${_vdt_real_lib}" DIRECTORY)
  get_filename_component(_vdt_prefix "${_vdt_lib_dir}" DIRECTORY)
  set(VDT_INCLUDE_DIR
      "${_vdt_prefix}/include"
      CACHE PATH "Path to VDT include directory")
  unset(_vdt_prefix)
  unset(_vdt_lib_dir)
  unset(_vdt_real_lib)
endif()

# Step 3: delegate version detection and target creation to ROOT's upstream
# finder.  It skips find_path/find_library because the variables are already
# set.  Fall back to a minimal implementation when ROOT is not yet available.
if(DEFINED ROOT_CMAKE_DIR AND EXISTS "${ROOT_CMAKE_DIR}/modules/FindVdt.cmake")
  include("${ROOT_CMAKE_DIR}/modules/FindVdt.cmake")
else()
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Vdt REQUIRED_VARS VDT_INCLUDE_DIR
                                                      VDT_LIBRARY)
  if(VDT_FOUND AND NOT TARGET VDT::VDT)
    add_library(VDT::VDT SHARED IMPORTED)
    target_include_directories(VDT::VDT SYSTEM INTERFACE "${VDT_INCLUDE_DIR}")
    set_target_properties(VDT::VDT PROPERTIES IMPORTED_LOCATION
                                              "${VDT_LIBRARY}")
  endif()
endif()
