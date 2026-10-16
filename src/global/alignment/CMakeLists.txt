get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets with default
# includes, libraries, and installation paths.
plugin_add(${PLUGIN_NAME} PLUGIN_USE_CC_ONLY)

# Pick up alignment.cc (and any future sources) automatically.
plugin_glob_all(${PLUGIN_NAME})

# Dependencies shared with the tracking plugin
plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

# Link against the tracking algorithm library so MeasurementToMille is
# available.
plugin_link_libraries(${PLUGIN_NAME} algorithms_tracking_library)

# NOTE: The edm4eic::AlignmentDerivativeSet type is defined in the alignment
# branch of EDM4eic (~/git/EDM4eic/build-align/edm4eic/).  Once it is merged
# into an official EDM4eic release and found by find_package(EDM4EIC), no extra
# include path is needed.  Until then, add the build directory manually:
#
# plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC
# $ENV{HOME}/git/EDM4eic/build-align)
