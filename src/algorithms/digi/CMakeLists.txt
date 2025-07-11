set(PLUGIN_NAME "algorithms_digi")

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} WITH_SHARED_LIBRARY WITHOUT_PLUGIN)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
plugin_add_algorithms(${PLUGIN_NAME})
plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

# Add include directories (same as target_include_directories but for both
# plugin and library) plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC
# ... )

# Add libraries (same as target_include_directories but for both plugin and
# library) plugin_link_libraries(${PLUGIN_NAME} ... )

# Link DD4hep library
plugin_link_libraries(${PLUGIN_NAME} DD4hep::DDDigi)
