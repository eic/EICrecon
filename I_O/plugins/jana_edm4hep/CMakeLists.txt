
cmake_minimum_required(VERSION 3.9)
project(jana_podio_project)

# Expose custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Set install directory to $JANA_HOME
set(CMAKE_INSTALL_PREFIX $ENV{JANA_HOME} CACHE PATH "magic incantation" FORCE)

# Find dependencies
find_package(JANA REQUIRED)
find_package(ROOT REQUIRED)

# The following cobbled from things found in the PODIO tests CMakeLists.txt
#set(CMAKE_MODULE_PATH $ENV{PODIO})  # Can't get this to work
set(podio_DIR $ENV{PODIO}/lib/cmake/podio)
find_package(podio REQUIRED)
PODIO_GENERATE_DATAMODEL(datamodel $ENV{EDM4HEP}/edm4hep.yaml podio_headers podio_sources IO_BACKEND_HANDLERS ${PODIO_IO_HANDLERS})
PODIO_ADD_DATAMODEL_CORE_LIB(TestDataModel "${podio_headers}" "${podio_sources}")
PODIO_ADD_ROOT_IO_DICT(TestDataModelDict TestDataModel "${podio_headers}" src/selection.xml)
set(PODIO_LIBRARIES TestDataModel TestDataModelDict podio::podioRootIO)

# This is used to automatically run the make_datamodel_glue.py command
# that generates the datamodel_glue.h file. This is temporary until
# a better mechanism using PODIO tools is developed.
add_custom_command(
        OUTPUT  datamodel_glue.h          # Treated as relative to CMAKE_CURRENT_BINARY_DIR
        COMMAND python ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
        DEPENDS ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
)

# Automatically determine source file list.
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( jana_podio_PLUGIN_SOURCES ${mysourcefiles} datamodel_glue.h )

message(STATUS jana_podio_PLUGIN_SOURCES=${jana_podio_PLUGIN_SOURCES})

add_library(jana_podio_plugin SHARED ${jana_podio_PLUGIN_SOURCES})

target_include_directories(jana_podio_plugin PUBLIC ${CMAKE_SOURCE_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(jana_podio_plugin ${JANA_LIB} ${PODIO_LIBRARIES} ${ROOT_LIBRARIES})
set_target_properties(jana_podio_plugin PROPERTIES PREFIX "" OUTPUT_NAME "jana_podio" SUFFIX ".so")

install(TARGETS jana_podio_plugin DESTINATION plugins)

file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/jana_podio)

# For root dictionaries
file(GLOB my_pcms "${CMAKE_CURRENT_BINARY_DIR}/*.pcm")
install(FILES ${my_pcms} DESTINATION plugins)

