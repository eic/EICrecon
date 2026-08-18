# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# Find Arrow (optional for Arrow stream reader support)
find_package(Arrow QUIET)
if(Arrow_FOUND AND Arrow_VERSION VERSION_GREATER_EQUAL "10.0.0")
  set(EICRECON_HAS_ARROW TRUE)
  message(STATUS "Arrow ${Arrow_VERSION} found - enabling Arrow stream reader")
else()
  set(EICRECON_HAS_ARROW FALSE)
  message(STATUS "Arrow >= 10.0.0 not found - Arrow stream reader disabled")
endif()

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Remove Arrow-specific sources if Arrow is not available
if(NOT EICRECON_HAS_ARROW)
  get_target_property(PLUGIN_SOURCES ${PLUGIN_NAME}_plugin SOURCES)
  list(FILTER PLUGIN_SOURCES EXCLUDE REGEX ".*ArrowStream.*")
  set_target_properties(${PLUGIN_NAME}_plugin PROPERTIES SOURCES "${PLUGIN_SOURCES}")
  message(STATUS "Excluded Arrow stream reader sources from build")
endif()

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
  $<TARGET_NAME_IF_EXISTS:podio::podioArrow>
  $<TARGET_NAME_IF_EXISTS:Acts::PluginPodio>
  $<TARGET_NAME_IF_EXISTS:ActsPodioEdm>
  log_library)

# Add Arrow if available
if(EICRECON_HAS_ARROW)
  plugin_link_libraries(${PLUGIN_NAME} arrow_shared)
  plugin_compile_definitions(${PLUGIN_NAME} PRIVATE EICRECON_ENABLE_ARROW)
endif()

# Add ZeroMQ support for managed PODIO processor
plugin_add_zeromq(${PLUGIN_NAME})

# Add nlohmann/json for JSON message parsing
find_package(nlohmann_json REQUIRED)
plugin_link_libraries(${PLUGIN_NAME} nlohmann_json::nlohmann_json)
