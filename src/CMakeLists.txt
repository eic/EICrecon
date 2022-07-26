
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
