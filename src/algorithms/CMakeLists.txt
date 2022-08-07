cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

print_header(">>>>   L I B R A R Y :   algorithms    <<<<")       # Fancy printing

project(algorithms_prj)

# Find dependencies
find_package(JANA REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED)
find_package(fmt REQUIRED)

set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)
set(INCLUDE_DIRS ${JANA_INCLUDE_DIR} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${fmt_INCLUDE_DIR})

set(ALGORITHMS_SOURCES
        trk_chain_example/v1/SimTrackerHitsCollectorT.h
        trk_chain_example/v1/RawTrackerHit.h
        trk_chain_example/v1/SiliconTracerDigiConfig.h
        trk_chain_example/v1/SiliconTrackerDigiT.h
        trk_chain_example/v1/SimTrackerHitsCollectorConfig.h
        trk_chain_example/v1/SimTrackerHitsCollectorT.h
        trk_chain_example/v1/TrackerHitReconstructionConfig.h
        trk_chain_example/v1/TrackerHitReconstructionT.h

        )

add_library(algorithms_lib STATIC ${ALGORITHMS_SOURCES})
target_include_directories(algorithms_lib SYSTEM PUBLIC ${INCLUDE_DIRS} )
target_include_directories(algorithms_lib PUBLIC ${CMAKE_SOURCE_DIR})
set_target_properties(algorithms_lib PROPERTIES PREFIX "lib" OUTPUT_NAME "algorithms" SUFFIX ".a")

install(TARGETS algorithms_lib DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY})
install(FILES ${HEADER_FILES} DESTINATION include/algorithms)


#message(STATUS "BEMC: {PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}")
#message(STATUS "BEMC: {CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/services/geometry/")
#message(STATUS "BEMC: {PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}")
#string(REPLACE ${CMAKE_SOURCE_DIR} "" PROJECT_RELATIVE_PATH ${PROJECT_SOURCE_DIR})
#message(STATUS "BEMC: {PROJECT_RELATIVE_PATH} ${PROJECT_RELATIVE_PATH}")
#
#
#
#
## Define library
#a
#target_include_directories(${_name}_library PUBLIC ${CMAKE_SOURCE_DIR})
#target_include_directories(${_name}_library SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
#set_target_properties(${_name}_library PROPERTIES PREFIX "lib" OUTPUT_NAME "${_name}" SUFFIX ".a")
#
## Define plugin
#add_library(${_name}_plugin SHARED ${PLUGIN_SOURCES})
#target_include_directories(${_name}_plugin PUBLIC ${CMAKE_SOURCE_DIR})
#target_include_directories(${_name}_plugin SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
#set_target_properties(${_name}_plugin PROPERTIES PREFIX "" OUTPUT_NAME "${_name}" SUFFIX ".so")
#
## Install plugin and library
#install(TARGETS ${_name}_plugin DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
#install(TARGETS ${_name}_library DESTINATION ${PLUGIN_LIBRARY_OUTPUT_DIRECTORY})
#
#
#
## Source files for plugin only (not library)
#set(PLUGIN_SOURCES BEMC.cc)
#
## Compile all other sources into a library
#file(GLOB LIBRARY_SOURCES *.cpp *.cc *.c  *.hpp *.hh *.h)
#list(REMOVE_ITEM LIBRARY_SOURCES ${PLUGIN_SOURCES})
#
#set( INCLUDE_DIRS ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${fmt_INCLUDE_DIR})
#
## Define library
#add_library( BEMC_library STATIC ${LIBRARY_SOURCES} )
#target_include_directories( BEMC_library PUBLIC ${INCLUDE_DIRS} )
#target_include_directories(BEMC_library PUBLIC ${CMAKE_SOURCE_DIR})
#target_link_libraries(BEMC_library ${JANA_LIB})
#set_target_properties(BEMC_library PROPERTIES PREFIX "lib" OUTPUT_NAME "BEMC" SUFFIX ".so")
#
## Define plugin
#add_library(BEMC_plugin SHARED ${PLUGIN_SOURCES})
#target_include_directories(BEMC_plugin PUBLIC ${INCLUDE_DIRS})
#target_include_directories(BEMC_plugin PUBLIC ${CMAKE_SOURCE_DIR})
#target_link_libraries(BEMC_plugin BEMC_library ${JANA_LIB})
#set_target_properties(BEMC_plugin PROPERTIES PREFIX "" OUTPUT_NAME "BEMC" SUFFIX ".so")
#
## Install plugin
#install(TARGETS BEMC_plugin DESTINATION plugins)
#install(TARGETS BEMC_library DESTINATION lib)
#
## Install headers for plugin
#file(GLOB ALL_HEADERS "*.h*")
#install(FILES ${ALL_HEADERS} DESTINATION include/detectors/BEMC)
