#  Variables to control this CMake build:
# ===================================================================================
#    CMake flag       |  environment variable  |  Description
# ===================================================================================
# -DEIC_DD4HEP_HOME   | EIC_DD4HEP_HOME        |  DD4Hep Detector geometry install
# -DIP6_DD4HEP_HOME   | IP6_DD4HEP_HOME        |  DD4Hep IP6 geometry install dir
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
# fmt                 |   Formatting library
# ==============================================================



cmake_minimum_required(VERSION 3.16)

project(EICRecon VERSION 0.0.0)

# Make C++17 a default
if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
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


# Check and print what JANA2 is used
find_package(JANA REQUIRED)
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 CMake   : ${JANA_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 includes: ${JANA_INCLUDE_DIR}")
message(STATUS "${CMAKE_PROJECT_NAME}: JANA2 library : ${JANA_LIBRARY}")


# Set it ON for additional CMake printout
set(EICRECON_VERBOSE_CMAKE OFF)


# Add CMake additional functionality:
include(cmake/print_functions.cmake)                        # Helpers to print fancy headers, file names, etc
include(cmake/print_subdirectory_tree.cmake)                # Prints processed subdirectories
include(cmake/jana_plugin.cmake)                            # Add common settings for plugins
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)   # Find Find<Modules>.cmake


# ------------------------------------------------------------------
print_grand_header("    B U I L D   E I C R E C O N   P A R T S    ")
# ------------------------------------------------------------------
add_subdirectory( src/algorithms )
add_subdirectory( src/benchmarks )
add_subdirectory( src/detectors )
add_subdirectory( src/examples )
add_subdirectory( src/global )
add_subdirectory( src/scripts )
add_subdirectory( src/services )
add_subdirectory( src/tests )
add_subdirectory( src/utilities )
add_subdirectory( src/benchmarks )


# Print what we had built
print_header("CMake processed subdirectories:")
print_subdirectory_tree()
message(STATUS "\n-------------------------------")


# Install all cmake helpers
include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/EICreconConfig.cmake.in cmake/EICreconConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)
install(FILES ${CMAKE_BINARY_DIR}/cmake/EICreconConfig.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)  # why is this needed?
file(GLOB EICRECON_CMAKE_FILES cmake/*.cmake)
install(FILES ${EICRECON_CMAKE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/EICrecon)

