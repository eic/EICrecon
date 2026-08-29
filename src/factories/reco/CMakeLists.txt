set(PLUGIN_NAME "factories_reco")
plugin_headers_only(${PLUGIN_NAME})

# Generate precompiled factory instantiations
generate_factory_precompile_sources(${PLUGIN_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
