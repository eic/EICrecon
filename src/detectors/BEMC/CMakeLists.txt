cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

project(BEMC_project)

# Find dependencies
find_package(JANA REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED)
find_package(fmt REQUIRED)

message(STATUS "BEMC: {PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}")
message(STATUS "BEMC: {CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/services/geometry/")



set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)

# Source files for plugin only (not library)
set(PLUGIN_SOURCES BEMC.cc)

# Compile all other sources into a library
file(GLOB LIBRARY_SOURCES *.cpp *.cc *.c  *.hpp *.hh *.h)
list(REMOVE_ITEM LIBRARY_SOURCES ${PLUGIN_SOURCES})

set( INCLUDE_DIRS ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${fmt_INCLUDE_DIR})

# Define library
add_library( BEMC_library STATIC ${LIBRARY_SOURCES} )
target_include_directories( BEMC_library PUBLIC ${INCLUDE_DIRS} )
target_include_directories(BEMC_library PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(BEMC_library ${JANA_LIB})

# Define plugin
add_library(BEMC_plugin SHARED ${PLUGIN_SOURCES})
target_include_directories(BEMC_plugin PUBLIC ${INCLUDE_DIRS})
target_include_directories(BEMC_plugin PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(BEMC_plugin BEMC_library ${JANA_LIB})
set_target_properties(BEMC_plugin PROPERTIES PREFIX "" OUTPUT_NAME "BEMC" SUFFIX ".so")

# Install plugin
install(TARGETS BEMC_plugin DESTINATION plugins)

# Install headers for plugin
file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/detectors/BEMC)


