cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

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

target_include_directories(BEMC_test_plugin PUBLIC ${CMAKE_SOURCE_DIR}/src ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} )
target_link_libraries(BEMC_test_plugin ${JANA_LIB} ${ROOT_LIBRARIES})
set_target_properties(BEMC_test_plugin PROPERTIES PREFIX "" OUTPUT_NAME "BEMC_test" SUFFIX ".so")

install(TARGETS BEMC_test_plugin DESTINATION plugins)

file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/BEMC_test)

# For root dictionaries
file(GLOB my_pcms "${CMAKE_CURRENT_BINARY_DIR}/*.pcm")
install(FILES ${my_pcms} DESTINATION plugins)

