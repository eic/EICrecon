cmake_minimum_required(VERSION 3.16)

# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

project(BEMC_test_project)

# Find dependencies
find_package(JANA REQUIRED)
find_package(ROOT REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)

# Automatically determine source file list.
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( BEMC_test_PLUGIN_SOURCES ${mysourcefiles} )

add_library(BEMC_test_plugin SHARED ${BEMC_test_PLUGIN_SOURCES})

target_include_directories(BEMC_test_plugin PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(BEMC_test_plugin SYSTEM PUBLIC ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} )
target_link_libraries(BEMC_test_plugin ${JANA_LIB} ${ROOT_LIBRARIES})
set_target_properties(BEMC_test_plugin PROPERTIES PREFIX "" OUTPUT_NAME "BEMC_test" SUFFIX ".so")

install(TARGETS BEMC_test_plugin DESTINATION plugins)

file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/BEMC_test)

# For root dictionaries
file(GLOB my_pcms "${CMAKE_CURRENT_BINARY_DIR}/*.pcm")
install(FILES ${my_pcms} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})

