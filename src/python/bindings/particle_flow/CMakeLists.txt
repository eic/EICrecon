# Python extension module eicrecon._particle_flow: the particle flow factory
# types (src/factories/particle_flow) for eicrecon.factories.particle_flow. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_particle_flow")
plugin_add_python_factories(${PLUGIN_NAME} particle_flow)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

plugin_link_libraries(${PLUGIN_NAME} algorithms_particle_flow_library
                      particle_service_library dd4hep_library log_library)
