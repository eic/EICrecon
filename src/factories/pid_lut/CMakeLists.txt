set(PLUGIN_NAME "factories_pid_lut")

# Glob precompile sources - each *_factory.cc provides explicit instantiation
file(GLOB FACTORY_PRECOMPILE_SOURCES "*_factory.cc")

# Create precompile library (compiled once with full factory definitions)
add_library(${PLUGIN_NAME}_precompile ${FACTORY_PRECOMPILE_SOURCES})
target_compile_definitions(${PLUGIN_NAME}_precompile PRIVATE EICRECON_FACTORY_PRECOMPILE)
target_link_libraries(${PLUGIN_NAME}_precompile PUBLIC
  algorithms_pid_lut_library
  extensions_jana_library
)

# Main plugin (header-only, fast compilation with forward declarations)
plugin_headers_only(${PLUGIN_NAME})
target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_NAME}_precompile)
