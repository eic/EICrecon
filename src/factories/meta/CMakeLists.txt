set(PLUGIN_NAME "factories_meta")

# Glob precompile sources - each *_factory.cc provides explicit instantiation
file(GLOB FACTORY_PRECOMPILE_SOURCES "*_factory.cc")

# Create precompile library (compiled once with full factory definitions)
add_library(${PLUGIN_NAME}_precompile ${FACTORY_PRECOMPILE_SOURCES})
target_compile_definitions(${PLUGIN_NAME}_precompile PRIVATE EICRECON_FACTORY_PRECOMPILE)
target_link_libraries(${PLUGIN_NAME}_precompile PUBLIC
  extensions_jana_library
)
# Meta algorithms are header-only, add include paths directly
target_include_directories(${PLUGIN_NAME}_precompile PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/algorithms>
)

# Install headers (plugin itself is header-only with forward declarations)
plugin_headers_only(${PLUGIN_NAME})
