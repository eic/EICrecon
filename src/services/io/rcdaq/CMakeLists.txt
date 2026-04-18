# Optional rcdaq binary-format event source plugin. The plugin is only built
# when the rcdaq format headers are found. Point cmake to the headers via:
# -Drcdaq_INCLUDE_DIR=/path/to/rcdaq

# ---- Locate rcdaq format headers ----------------------------------------
find_path(
  rcdaq_INCLUDE_DIR
  NAMES EvtStructures.h
  HINTS ${rcdaq_INCLUDE_DIR} $ENV{rcdaq_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/../rcdaq /opt/rcdaq/include
  DOC "Directory containing rcdaq format headers (EvtStructures.h etc.)")

if(NOT rcdaq_INCLUDE_DIR)
  message(STATUS "rcdaq headers not found — skipping io/rcdaq plugin")
  return()
endif()

message(STATUS "Building io/rcdaq plugin (rcdaq headers: ${rcdaq_INCLUDE_DIR})")

# ---- Plugin target -------------------------------------------------------
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

plugin_add(${PLUGIN_NAME})
plugin_glob_all(${PLUGIN_NAME})

# Expose rcdaq format headers to this target only
plugin_include_directories(${PLUGIN_NAME} PRIVATE ${rcdaq_INCLUDE_DIR})

plugin_link_libraries(${PLUGIN_NAME} fmt::fmt podio::podioIO log_library)
