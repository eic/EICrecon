# Automatically set plugin name the same as the directory name Don't forget
# string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in
# its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Creates ${PLUGIN_NAME}_plugin: the Python binding module
# eicrecon._wiring_<name> with the factory types used by
# src/python/eicrecon/plugins/<name>.py
plugin_add_python_wiring(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_eigen3(${PLUGIN_NAME})

# Add libraries (works same as target_include_directories)
plugin_link_libraries(${PLUGIN_NAME} algorithms_calorimetry_library
                      algorithms_digi_library log_library)
