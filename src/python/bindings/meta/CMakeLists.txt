# Python extension module eicrecon._meta: the generic (meta) factory types
# (src/factories/meta) for eicrecon.factories.meta. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_meta")
plugin_add_python_factories(${PLUGIN_NAME} meta)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_algorithms(${PLUGIN_NAME})
plugin_add_onnxruntime(${PLUGIN_NAME})

plugin_link_libraries(
  ${PLUGIN_NAME} algorithms_onnx_library algorithms_reco_library
  particle_service_library log_library)
