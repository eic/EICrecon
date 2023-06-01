cmake_minimum_required(VERSION 3.16)

get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths

plugin_add(${PLUGIN_NAME} WITH_STATIC_LIBRARY)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_dd4hep(${PLUGIN_NAME})

# Add libraries (works same as target_include_directories)
plugin_link_libraries(${PLUGIN_NAME} calorimetry_algorithms_library)
