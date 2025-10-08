# cmake-format: off
#
# Variables to control this CMake build:
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
#
# cmake-format: on

cmake_minimum_required(VERSION 3.24)

# 3.27: find_package() uses upper-case <PACKAGENAME>_ROOT variables
cmake_policy(SET CMP0144 NEW)

project(EICrecon LANGUAGES CXX)

# CMake includes
include(CheckCXXCompilerFlag)

# Set version based on git
include(cmake/git_version.cmake)
set_git_version(CMAKE_PROJECT_VERSION)

# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

# Minimal dependency versions
set(Acts_VERSION_MIN 34.0.0)
set(algorithms_VERSION_MIN 1.0.0)
set(DD4hep_VERSION_MIN 1.21)
set(EDM4EIC_VERSION_MIN 7.0)
set(EDM4HEP_VERSION_MIN 0.7.1)
set(Eigen3_VERSION_MIN 3.3)
set(FastJet_VERSION_MIN 3)
set(FastJetContrib_VERSION_MIN 1.46)
set(fmt_VERSION_MIN 9.0.0)
set(IRT_VERSION_MIN 1.0.5)
set(JANA_VERSION_MIN 2.2.0)
set(onnxruntime_MIN_VERSION 1.17)
set(podio_VERSION_MIN 0.17)
set(ROOT_VERSION_MIN 6.28)
set(spdlog_VERSION_MIN 1.11.0)

# Set default standard to C++20
set(CMAKE_CXX_STANDARD_MIN 20)
set(CMAKE_CXX_STANDARD
    ${CMAKE_CXX_STANDARD_MIN}
    CACHE STRING "C++ standard to be used")
if(CMAKE_CXX_STANDARD LESS CMAKE_CXX_STANDARD_MIN)
  message(
    FATAL_ERROR
      "Unsupported C++ standard: ${CMAKE_CXX_STANDARD} (at least ${CMAKE_CXX_STANDARD_MIN} required)"
  )
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Infrastructure to support use of --no-as-needed
include(CheckLinkerFlag)
check_linker_flag(CXX "LINKER:--no-as-needed" CXX_LINKER_HAS_no_as_needed)
set(CMAKE_CXX_LINK_LIBRARY_USING_NO_AS_NEEDED_SUPPORTED TRUE)
set(CMAKE_CXX_LINK_LIBRARY_USING_NO_AS_NEEDED
    "LINKER:--push-state,--no-as-needed" "<LINK_ITEM>" "LINKER:--pop-state")

# Export compile commands as json for run-clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# This PROJECT_SOURCE_DIR at this point corresponds to repository root. Saving
# it as EICRECON_SOURCE_DIR so can be used for jana_plugin.cmake to correctly
# set plugin includes (and in other places as needed)
set(EICRECON_SOURCE_DIR ${PROJECT_SOURCE_DIR})

# Also use clang-tidy integration in CMake
option(ENABLE_CLANG_TIDY "Enable clang-tidy integration in cmake" OFF)
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE}"
        CACHE STRING "" FORCE)
  else()
    set(CMAKE_CXX_CLANG_TIDY
        ""
        CACHE STRING "" FORCE)
  endif()
endif()

# Enable -fPIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Install to the top directory by default
if(${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
  set(CMAKE_INSTALL_PREFIX
      ${CMAKE_SOURCE_DIR}
      CACHE PATH "Install in top directory by default" FORCE)
endif()

# Install to standard location
include(GNUInstallDirs)

# Default plugins installation directory is 'plugins'
if(NOT DEFINED PLUGIN_OUTPUT_DIRECTORY)
  set(PLUGIN_OUTPUT_DIRECTORY "lib/${PROJECT_NAME}/plugins")
  message(STATUS "${CMAKE_PROJECT_NAME}: Set default PLUGIN_OUTPUT_DIRECTORY")
endif()
message(
  STATUS
    "${CMAKE_PROJECT_NAME}: PLUGIN_OUTPUT_DIRECTORY: ${PLUGIN_OUTPUT_DIRECTORY}"
)

# Default plugins static libraries installation directory is 'lib'
if(NOT DEFINED PLUGIN_LIBRARY_OUTPUT_DIRECTORY)
  set(PLUGIN_LIBRARY_OUTPUT_DIRECTORY "lib")
  message(STATUS "${CMAKE_PROJECT_NAME}: Set default PLUGIN_OUTPUT_DIRECTORY")
endif()
message(
  STATUS
    "${CMAKE_PROJECT_NAME}: PLUGIN_LIBRARY_OUTPUT_DIRECTORY: ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY}"
)

# Use, i.e. don't skip the full RPATH for the build tree, and use relative paths
# for relocatable build products
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# When building, don't use the install RPATH already (but later on when
# installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH
    "${CMAKE_INSTALL_PREFIX}/${PLUGIN_LIBRARY_OUTPUT_DIRECTORY};${CMAKE_INSTALL_PREFIX}/${PLUGIN_OUTPUT_DIRECTORY}"
)

# Add the automatically determined parts of the RPATH which point to directories
# outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Check and print what JANA2 is used
find_package(JANA ${JANA_VERSION_MIN} REQUIRED)
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 CMake   : ${JANA_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 includes: ${JANA_INCLUDE_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 library : ${JANA_LIBRARY}")

# Algorithms
find_package(algorithms ${algorithms_VERSION_MIN} REQUIRED Core)

# PODIO, EDM4HEP, EDM4EIC event models
find_package(Eigen3 ${Eigen3_VERSION_MIN} REQUIRED)
find_package(podio ${podio_VERSION_MIN} QUIET)
if(NOT podio_FOUND)
  find_package(podio 1.0 REQUIRED)
endif()
find_package(EDM4HEP ${EDM4HEP_VERSION_MIN} REQUIRED)
find_package(EDM4EIC ${EDM4EIC_VERSION_MIN} REQUIRED)

# fmt
find_package(fmt ${fmt_VERSION_MIN} REQUIRED)

# spdlog
find_package(spdlog ${spdlog_VERSION_MIN} REQUIRED)

# Guidelines Support Library
find_package(Microsoft.GSL CONFIG)

# Remove PODIO_JSON_OUTPUT (ref: https://github.com/AIDASoft/podio/issues/475)
get_target_property(EDM4HEP_INTERFACE_COMPILE_DEFINITIONS EDM4HEP::edm4hep
                    INTERFACE_COMPILE_DEFINITIONS)
if(EDM4HEP_INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER EDM4HEP_INTERFACE_COMPILE_DEFINITIONS EXCLUDE REGEX
       [[^PODIO_JSON_OUTPUT$]])
  set_property(
    TARGET EDM4HEP::edm4hep PROPERTY INTERFACE_COMPILE_DEFINITIONS
                                     ${EDM4HEP_INTERFACE_COMPILE_DEFINITIONS})
endif()
get_target_property(EDM4EIC_INTERFACE_COMPILE_DEFINITIONS EDM4EIC::edm4eic
                    INTERFACE_COMPILE_DEFINITIONS)
if(EDM4EIC_INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER EDM4EIC_INTERFACE_COMPILE_DEFINITIONS EXCLUDE REGEX
       [[^PODIO_JSON_OUTPUT$]])
  set_property(
    TARGET EDM4EIC::edm4eic PROPERTY INTERFACE_COMPILE_DEFINITIONS
                                     ${EDM4EIC_INTERFACE_COMPILE_DEFINITIONS})
endif()

# DD4Hep is required for the most of the part
find_package(DD4hep ${DD4hep_VERSION_MIN} REQUIRED)

# ACTS cmake-lint: disable=C0103
find_package(Acts REQUIRED COMPONENTS Core PluginDD4hep PluginJson)
set(Acts_VERSION
    "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN})
  message(
    FATAL_ERROR
      "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found"
  )
endif()
if(${Acts_VERSION} VERSION_GREATER_EQUAL "43.0.0")
  set(Acts_NAMESPACE_PREFIX Acts::)
else()
  set(Acts_NAMESPACE_PREFIX Acts)
endif()
# Acts::Core
get_target_property(ACTS_COMPILE_FEATURES ${Acts_NAMESPACE_PREFIX}Core
                    INTERFACE_COMPILE_FEATURES)
if(NOT "cxx_std_${CMAKE_CXX_STANDARD}" IN_LIST ACTS_COMPILE_FEATURES)
  message(
    WARNING
      "Acts has not been built with the requested C++ standard ${CMAKE_CXX_STANDARD}."
  )
endif()

# ROOT
find_package(ROOT ${ROOT_VERSION_MIN} REQUIRED)
# Check that ROOT is compiled with a modern enough c++ standard
get_target_property(ROOT_COMPILE_FEATURES ROOT::Core INTERFACE_COMPILE_FEATURES)
if(NOT "cxx_std_${CMAKE_CXX_STANDARD}" IN_LIST ROOT_COMPILE_FEATURES)
  message(
    WARNING
      "ROOT has not been built with the requested C++ standard ${CMAKE_CXX_STANDARD}."
  )
endif()

# ONNX Runtime
find_package(onnxruntime ${onnxruntime_MIN_VERSION} CONFIG)

# Add CMake additional functionality:
include(cmake/jana_plugin.cmake) # Add common settings for plugins
list(APPEND CMAKE_MODULE_PATH ${EICRECON_SOURCE_DIR}/cmake
)# Find Find<Modules>.cmake

enable_testing()

# Address sanitizer
option(USE_ASAN "Compile with address sanitizer" OFF)
if(${USE_ASAN})
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O1)
  add_link_options(-fsanitize=address)
  install(FILES .github/asan.supp .github/lsan.supp
          DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
endif()

# Thread sanitizer
option(USE_TSAN "Compile with thread sanitizer" OFF)
if(${USE_TSAN})
  add_compile_options(-fsanitize=thread -g -O1)
  add_link_options(-fsanitize=thread)
  install(FILES .github/tsan.supp
          DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
endif()

# Undefined behavior sanitizer
option(USE_UBSAN "Compile with undefined behavior sanitizer" OFF)
if(${USE_UBSAN})
  foreach(sanitizer undefined float-divide-by-zero unsigned-integer-overflow
                    implicit-conversion local-bounds nullability)
    check_cxx_compiler_flag("-fsanitize=${sanitizer}"
                            CXX_COMPILER_HAS_sanitize_${sanitizer})
    if(CXX_COMPILER_HAS_sanitize_${sanitizer})
      add_compile_options(-fsanitize=${sanitizer})
      add_link_options(-fsanitize=${sanitizer})
    endif()
  endforeach()
  add_compile_options(-g -O1)
  install(FILES .github/ubsan.supp
          DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
endif()

# Compress debug symbols if supported
check_cxx_compiler_flag("-gz" CXX_COMPILER_HAS_gz)
if(CXX_COMPILER_HAS_gz)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gz")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -gz")
endif()

add_subdirectory(src/services)
add_subdirectory(src/algorithms)
add_subdirectory(src/benchmarks)
add_subdirectory(src/detectors)
add_subdirectory(src/extensions)
add_subdirectory(src/factories)
add_subdirectory(src/global)
add_subdirectory(src/scripts)
add_subdirectory(src/tests)
add_subdirectory(src/utilities)

# Install all cmake helpers
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${CMAKE_PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(
  EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(
  DIRECTORY cmake/
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  FILES_MATCHING
  PATTERN *.cmake)
