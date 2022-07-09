
cmake_minimum_required(VERSION 3.9)
project(EICrecon)

if(NOT "${CMAKE_CXX_STANDARD}")
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # Enable -fPIC for all targets


add_subdirectory( I_O/plugins/jana_edm4hep )

