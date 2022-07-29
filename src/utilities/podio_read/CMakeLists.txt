
cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

set(CMAKE_CXX_STANDARD 17)
project(podio_read_project)

# Find dependencies
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
set(PODIO_LIBRARIES EDM4HEP::edm4hep EDM4HEP::edm4hepDict podio::podioRootIO)

# This is used to automatically run the make_datamodel_glue.py command
# that generates the datamodel_glue.h file. This is temporary until
# a better mechanism using PODIO tools is developed.
#add_custom_command(
#        OUTPUT  datamodel_glue.h          # Treated as relative to CMAKE_CURRENT_BINARY_DIR
#        COMMAND python ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
#        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#        DEPENDS ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
#)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# Automatically determine source file list.
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( podio_read_SOURCES ${mysourcefiles} )

# Make plugin target
add_executable(podio_read ${podio_read_SOURCES})
target_include_directories(podio_read PUBLIC ${CMAKE_SOURCE_DIR} ${EDM4HEP_INCLUDE_DIR} ${PODIO_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(podio_read ${JANA_LIB} ${PODIO_LIBRARIES})


