
cmake_minimum_required(VERSION 3.16)

# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

project(podio_test_project)

# Find dependencies
find_package(JANA REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)

# Automatically determine source file list.
file(GLOB mysourcefiles *.cpp *.cc *.c  *.hpp *.hh *.h)
set( podio_test_SOURCES ${mysourcefiles} )

add_executable(podio_test ${podio_test_SOURCES})

target_include_directories(podio_test PUBLIC ${EICRECON_SOURCE_DIR}/src)
target_include_directories(podio_test SYSTEM PUBLIC ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} )
target_link_libraries(podio_test ${JANA_LIB} ${ROOT_LIBRARIES} podio_plugin log_plugin)

install(TARGETS podio_test DESTINATION bin)

file(GLOB my_headers "*.h*")
install(FILES ${my_headers} DESTINATION include/podio_test)

# For root dictionaries
# file(GLOB my_pcms "${CMAKE_CURRENT_BINARY_DIR}/*.pcm")
# install(FILES ${my_pcms} DESTINATION bin)
