# Python extension module eicrecon._tracking: the tracking factory types
# (src/factories/tracking) for eicrecon.factories.tracking. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_tracking")
plugin_add_python_factories(${PLUGIN_NAME} tracking)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_eigen3(${PLUGIN_NAME})

plugin_link_libraries(${PLUGIN_NAME} algorithms_tracking_library acts_library
                      evaluator_library dd4hep_library log_library)
