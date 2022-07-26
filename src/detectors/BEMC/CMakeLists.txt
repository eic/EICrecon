cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

project(BarrelEMCal_project)

# Set install directory to $JANA_HOME
# TODO: This needs to inherit from the top-level CMakeLists.txt
set(CMAKE_INSTALL_PREFIX $ENV{JANA_HOME} CACHE PATH "magic incantation" FORCE)

# Find dependencies
find_package(JANA REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED)
find_package(fmt REQUIRED)

set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# Compile all sources
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( BarrelEMCal_PLUGIN_SOURCES ${mysourcefiles} )

# Define plugin
add_library(BarrelEMCal_plugin SHARED ${BarrelEMCal_PLUGIN_SOURCES})
target_include_directories(BarrelEMCal_plugin PUBLIC ${CMAKE_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${fmt_INCLUDE_DIR})
target_link_libraries(BarrelEMCal_plugin ${JANA_LIB})
set_target_properties(BarrelEMCal_plugin PROPERTIES PREFIX "" OUTPUT_NAME "BarrelEMCal" SUFFIX ".so")

# Install plugin
install(TARGETS BarrelEMCal_plugin DESTINATION plugins)

# Install headers for plugin
file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/BarrelEMCal)


