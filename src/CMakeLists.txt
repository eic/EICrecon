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



cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

project(EICrecon)

if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # Enable -fPIC for all targets

if( ${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT} )
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/.. CACHE PATH "Install in parent directory of src by default" FORCE)
endif()



add_subdirectory( detectors/BEMC )

add_subdirectory( services/geometry/dd4hep )
add_subdirectory( services/io/podio )
