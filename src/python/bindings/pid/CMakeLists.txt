# Python extension module eicrecon._pid: the particle identification factory
# types (src/factories/pid) for eicrecon.factories.pid. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_pid")
plugin_add_python_factories(${PLUGIN_NAME} pid)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_irt(${PLUGIN_NAME})
# IRT2 is optional: the RICH-IRT factory types and helpers are compiled only
# with it
plugin_add_irt2(${PLUGIN_NAME})

plugin_link_libraries(
  ${PLUGIN_NAME}
  algorithms_pid_library
  algorithms_tracking_library
  richgeo_library
  acts_library
  particle_service_library
  dd4hep_library
  log_library)
