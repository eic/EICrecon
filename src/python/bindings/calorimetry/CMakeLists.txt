# Python extension module eicrecon._calorimetry: the calorimetry factory types
# (src/factories/calorimetry) for eicrecon.factories.calorimetry. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_calorimetry")
plugin_add_python_factories(${PLUGIN_NAME} calorimetry)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

plugin_link_libraries(${PLUGIN_NAME} algorithms_calorimetry_library
                      algorithms_onnx_library dd4hep_library log_library)
