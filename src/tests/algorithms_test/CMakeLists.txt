# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(DIRECOTRY_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
#plugin_add(${PLUGIN_NAME})

# Find dependencies
find_package(JANA REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO)

## The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
## Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
## Adds headers to the correct installation directory
#plugin_glob_all(${PLUGIN_NAME})
#
#
## Add include directories
## (same as target_include_directories but for both plugin and library)
#plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${JANA_INCLUDE_DIR} ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})
#
## Add libraries
## (same as target_include_directories but for both plugin and library)
#plugin_link_libraries(${PLUGIN_NAME} ${JANA_LIB} ${ROOT_LIBRARIES})



find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(${DIRECOTRY_NAME} algorithms_test.cc)
target_link_libraries(${DIRECOTRY_NAME} PRIVATE Catch2::Catch2WithMain)

# These tests need their own main
#add_executable(custom-main-tests test.cpp test-main.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)

# Install executable
install(TARGETS ${DIRECOTRY_NAME} DESTINATION bin)
