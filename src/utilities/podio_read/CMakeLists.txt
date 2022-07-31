
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

# Reading data from the events tree will fail with complaints about the
# vector<edm4hep::EventHeader> and similar not being in the dictionary.
# Generate a ROOT dictionary here to supplement the one created by the
# edn4hep source code. This one simply defines these vector types.
# TODO: Add a make_datamodel_glue.py script to generate the header
# TODO: and LinkDef files automatically for all edm4hep data types.
include_directories(${EDM4HEP_INCLUDE_DIR} ${podio_INCLUDE_DIR})
root_generate_dictionary(G__edm4hep_vectors edm4hep_vectors.h LINKDEF edm4hep_vectors_LinkDef.h)

# Automatically determine source file list.
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( podio_read_SOURCES ${mysourcefiles} G__edm4hep_vectors.cxx)

# Make  target
add_executable(podio_read ${podio_read_SOURCES})
target_include_directories(podio_read PUBLIC ${CMAKE_SOURCE_DIR} ${EDM4HEP_INCLUDE_DIR} ${PODIO_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(podio_read ${JANA_LIB} ${PODIO_LIBRARIES})

