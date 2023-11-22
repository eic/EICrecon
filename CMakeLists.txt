#  Variables to control this CMake build:
# ===================================================================================
#    CMake flag       |  environment variable  |  Description
# ===================================================================================
# -- (under review)   | EDM4HEP_ROOT           |  Edm4Hep  installation directory
#------------------------------------------------------------------------------------
# -DCMAKE_CXX_STANDARD      | 17 - standard for C++ compilation
# ===================================================================================
#
#
# ==============================================================
#    DEPENDENCIES (used by find_package)
# ==============================================================
#      Name           |  Description
# ==============================================================
# JANA                |   Jana2 framework
# EDM4HEP             |   Event data model based on podio
# podio               |   IO library
# DD4hep              |   Geometry framework
# ROOT                |   CERN ROOT
# spdlog              |   Formatting library
# IRT                 |   Indirect Ray Tracing library
# ==============================================================



cmake_minimum_required(VERSION 3.18)

project(EICRecon)

# CMake includes
include(CheckCXXCompilerFlag)

# Set version based on git
include(cmake/git_version.cmake)
set_git_version(CMAKE_PROJECT_VERSION)

# Set default standard to C++20
set(CMAKE_CXX_STANDARD_MIN 20)
set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD_MIN} CACHE STRING "C++ standard to be used")
if(CMAKE_CXX_STANDARD LESS CMAKE_CXX_STANDARD_MIN)
  message(FATAL_ERROR "Unsupported C++ standard: ${CMAKE_CXX_STANDARD} (at least ${CMAKE_CXX_STANDARD_MIN} required)")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands as json for run-clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# This PROJECT_SOURCE_DIR at this point corresponds to repository root.
# Saving it as EICRECON_SOURCE_DIR so can be used for jana_plugin.cmake
# to correctly set plugin includes (and in other places as needed)
set(EICRECON_SOURCE_DIR ${PROJECT_SOURCE_DIR})

# Also use clang-tidy integration in CMake
option(ENABLE_CLANG_TIDY "Enable clang-tidy integration in cmake" OFF)
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if (CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" CACHE STRING "" FORCE)
  else()
    set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE)
  endif()
endif()

# Enable -fPIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Install to the top directory by default
if( ${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT} )
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "Install in top directory by default" FORCE)
endif()

# Default plugins installation directory is 'plugins'
if(NOT DEFINED PLUGIN_OUTPUT_DIRECTORY)
    set(PLUGIN_OUTPUT_DIRECTORY "lib/EICrecon/plugins")
    message(STATUS "${CMAKE_PROJECT_NAME}: Set default PLUGIN_OUTPUT_DIRECTORY")
endif()
message(STATUS "${CMAKE_PROJECT_NAME}: PLUGIN_OUTPUT_DIRECTORY: ${PLUGIN_OUTPUT_DIRECTORY}")

# Default plugins static libraries installation directory is 'lib'
if(NOT DEFINED PLUGIN_LIBRARY_OUTPUT_DIRECTORY)
    set(PLUGIN_LIBRARY_OUTPUT_DIRECTORY "lib")
    message(STATUS "${CMAKE_PROJECT_NAME}: Set default PLUGIN_OUTPUT_DIRECTORY")
endif()
message(STATUS "${CMAKE_PROJECT_NAME}: PLUGIN_LIBRARY_OUTPUT_DIRECTORY: ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY}")


# Use, i.e. don't skip the full RPATH for the build tree,
# and use relative paths for relocatable build products
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# When building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH
    "${CMAKE_INSTALL_PREFIX}/${PLUGIN_LIBRARY_OUTPUT_DIRECTORY};${CMAKE_INSTALL_PREFIX}/${PLUGIN_OUTPUT_DIRECTORY}"
)

# Add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# Check and print what JANA2 is used
find_package(JANA 2.0.8 REQUIRED)
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 CMake   : ${JANA_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 includes: ${JANA_INCLUDE_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 library : ${JANA_LIBRARY}")
add_compile_definitions(HAVE_PODIO)
# TODO: NWB: Do this correctly in JANA via target_compile_definitions for v2.1.1
# TODO: NWB: Maybe we want these to be prefixed, e.g. JANA2_HAVE_PODIO

# Algorithms
find_package(algorithms 1.0.0 REQUIRED Core)

# PODIO, EDM4HEP, EDM4EIC event models
find_package(Eigen3 REQUIRED)
find_package(podio REQUIRED)
find_package(EDM4HEP 0.7.1 REQUIRED)
find_package(EDM4EIC 2.1 REQUIRED)

# Guidelines Support Library
find_package(Microsoft.GSL CONFIG)

# Remove PODIO_JSON_OUTPUT (ref: https://github.com/AIDASoft/podio/issues/475)
get_target_property(EDM4HEP_INTERFACE_COMPILE_DEFINITIONS EDM4HEP::edm4hep INTERFACE_COMPILE_DEFINITIONS)
if(EDM4HEP_INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER EDM4HEP_INTERFACE_COMPILE_DEFINITIONS EXCLUDE REGEX [[^PODIO_JSON_OUTPUT$]])
  set_property(TARGET EDM4HEP::edm4hep PROPERTY INTERFACE_COMPILE_DEFINITIONS ${EDM4HEP_INTERFACE_COMPILE_DEFINITIONS})
endif()
get_target_property(EDM4EIC_INTERFACE_COMPILE_DEFINITIONS EDM4EIC::edm4eic INTERFACE_COMPILE_DEFINITIONS)
if(EDM4EIC_INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER EDM4EIC_INTERFACE_COMPILE_DEFINITIONS EXCLUDE REGEX [[^PODIO_JSON_OUTPUT$]])
  set_property(TARGET EDM4EIC::edm4eic PROPERTY INTERFACE_COMPILE_DEFINITIONS ${EDM4EIC_INTERFACE_COMPILE_DEFINITIONS})
endif()

# DD4Hep is required for the most of the part
find_package(DD4hep REQUIRED)

# ACTS
find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginDD4hep)
set(Acts_VERSION_MIN "19.0.0")
set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
        AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
    message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
endif()

# CERN ROOT
find_package(ROOT REQUIRED)

# Set it ON for additional CMake printout
set(EICRECON_VERBOSE_CMAKE OFF)


# Add CMake additional functionality:
include(cmake/print_functions.cmake)                        # Helpers to print fancy headers, file names, etc
include(cmake/jana_plugin.cmake)                            # Add common settings for plugins
list (APPEND CMAKE_MODULE_PATH ${EICRECON_SOURCE_DIR}/cmake)   # Find Find<Modules>.cmake

enable_testing()

# Address sanitizer
option(USE_ASAN "Compile with address sanitizer" OFF)
if (${USE_ASAN})
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=address)
endif()

# Thread sanitizer
option(USE_TSAN "Compile with thread sanitizer" OFF)
if (${USE_TSAN})
    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif()

# Undefined behavior sanitizer
option(USE_UBSAN "Compile with undefined behavior sanitizer" OFF)
if (${USE_UBSAN})
    foreach (sanitizer undefined float-divide-by-zero unsigned-integer-overflow implicit-conversion local-bounds nullability)
        check_cxx_compiler_flag("-fsanitize=${sanitizer}" CXX_COMPILER_HAS_sanitize_${sanitizer})
        if (CXX_COMPILER_HAS_sanitize_${sanitizer})
            add_compile_options(-fsanitize=${sanitizer})
            add_link_options(-fsanitize=${sanitizer})
        endif()
    endforeach()
    add_compile_options(-g -O1)
endif()

# Compress debug symbols if supported
check_cxx_compiler_flag("-gz" CXX_COMPILER_HAS_gz)
if (CXX_COMPILER_HAS_gz)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gz")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -gz")
endif()

add_subdirectory( src/services )
add_subdirectory( src/algorithms )
add_subdirectory( src/benchmarks )
add_subdirectory( src/detectors )
add_subdirectory( src/examples )
add_subdirectory( src/global )
add_subdirectory( src/scripts )
add_subdirectory( src/tests )
add_subdirectory( src/utilities )

# Install all cmake helpers
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/EICreconConfig.cmake.in cmake/EICreconConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)
install(FILES ${CMAKE_BINARY_DIR}/cmake/EICreconConfig.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)  # why is this needed?
file(GLOB EICRECON_CMAKE_FILES cmake/*.cmake)
install(FILES ${EICRECON_CMAKE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)
