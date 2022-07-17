
cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

project(EICrecon)

if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # Enable -fPIC for all targets


add_subdirectory( I_O/plugins/jana_edm4hep )
add_subdirectory( GEOMETRY/plugins/jana_dd4hep )

add_subdirectory( RECON/plugins/BarrelEMCal )
