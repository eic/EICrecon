# Python extension module eicrecon._digi: the digitization factory types
# (src/factories/digi) for eicrecon.factories.digi. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_digi")
plugin_add_python_factories(${PLUGIN_NAME} digi)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})

plugin_link_libraries(
  ${PLUGIN_NAME}
  algorithms_digi_library
  algorithms_calorimetry_library
  algorithms_tracking_library
  acts_library
  richgeo_library
  dd4hep_library
  log_library)
