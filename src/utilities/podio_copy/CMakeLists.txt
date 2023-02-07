cmake_minimum_required(VERSION 3.16)

project(podio_copy_project)

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

# Automatically determine source file list.
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( podio_copy_SOURCES ${mysourcefiles} )

# Make plugin target
add_executable(podio_copy ${podio_copy_SOURCES})
target_include_directories(podio_copy PUBLIC ${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(podio_copy ${JANA_LIB} ${PODIO_LIBRARIES})
