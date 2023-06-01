cmake_minimum_required(VERSION 3.16)

project(TRACKINGcheck)

# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} )

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
find_package(Eigen3 REQUIRED)
plugin_add_acts(${PLUGIN_NAME})
plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

set(Eigen3_INCLUDE_DIR ${Eigen3_DIR}/../../../include/eigen3)

# Add include directories (works same as target_include_directories)
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${Eigen3_INCLUDE_DIR})

# Add libraries (works same as target_include_directories)
# plugin_link_libraries(${PLUGIN_NAME} ... )
