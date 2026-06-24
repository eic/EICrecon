# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories (works same as target_include_directories)
plugin_include_directories(${PLUGIN_NAME} PRIVATE ${PROJECT_BINARY_DIR}/include)

# Add libraries (works same as target_include_directories)
plugin_link_libraries(
  ${PLUGIN_NAME}
  fmt::fmt
  EDM4HEP::edm4hep
  EDM4HEP::edm4hepDict
  EDM4EIC::edm4eic
  EDM4EIC::edm4eic_utils
  podio::podioIO
  $<TARGET_NAME_IF_EXISTS:${Acts_NAMESPACE_PREFIX}PluginPodio>
  $<TARGET_NAME_IF_EXISTS:ActsPodioEdm>
  log_library)

# Add ZeroMQ support for managed PODIO processor
plugin_add_zeromq(${PLUGIN_NAME})

# Add nlohmann/json for JSON message parsing
find_package(nlohmann_json REQUIRED)
plugin_link_libraries(${PLUGIN_NAME} nlohmann_json::nlohmann_json)
