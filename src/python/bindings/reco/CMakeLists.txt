# Python extension module eicrecon._reco: the reconstruction factory types
# (src/factories/reco) for eicrecon.factories.reco. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_reco")
plugin_add_python_factories(${PLUGIN_NAME} reco)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_onnxruntime(${PLUGIN_NAME})

plugin_link_libraries(
  ${PLUGIN_NAME} algorithms_reco_library algorithms_onnx_library
  particle_service_library dd4hep_library log_library)
