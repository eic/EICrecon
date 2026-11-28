# Python extension module eicrecon._fardetectors: the far-forward detector
# factory types (src/factories/fardetectors) for
# eicrecon.factories.fardetectors. See plugin_add_python_factories in
# cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_fardetectors")
plugin_add_python_factories(${PLUGIN_NAME} fardetectors)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_eigen3(${PLUGIN_NAME})

plugin_link_libraries(${PLUGIN_NAME} algorithms_fardetectors_library
                      dd4hep_library log_library)
