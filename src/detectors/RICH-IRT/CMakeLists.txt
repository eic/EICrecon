# Automatically set plugin name the same as the directory name Don't forget
# string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in
# its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# The wiring is compiled only with IRT2 (>= IRT2_VERSION_MIN): without it, the
# binding module would have no entry point and fail to import (e.g. in wiring
# capture, which imports every binding module), so it is not built at all.
# eicrecon.plugins.RICH_IRT then wires nothing.
find_package(IRT2 ${IRT2_VERSION_MIN} QUIET)
if(NOT IRT2_FOUND)
  return()
endif()
# plugin_add_irt2() below sets up IRT2 for the target only if not yet found
unset(IRT2_FOUND)

# Creates ${PLUGIN_NAME}_plugin: the Python binding module
# eicrecon._wiring_<name> with the factory types used by
# src/python/eicrecon/plugins/<name>.py
plugin_add_python_wiring(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
plugin_add_irt(${PLUGIN_NAME})
plugin_add_irt2(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})

plugin_link_libraries(
  ${PLUGIN_NAME} algorithms_pid_library algorithms_tracking_library log_library
  dd4hep_library acts_library)
