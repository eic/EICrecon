# Python extension module eicrecon._pid_lut: the PID lookup table factory types
# (src/factories/pid_lut) for eicrecon.factories.pid_lut. See
# plugin_add_python_factories in cmake/jana_plugin.cmake.
set(PLUGIN_NAME "python_pid_lut")
plugin_add_python_factories(${PLUGIN_NAME} pid_lut)
plugin_glob_all(${PLUGIN_NAME})

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

plugin_link_libraries(${PLUGIN_NAME} algorithms_pid_lut_library pid_lut_library
                      particle_service_library log_library)
