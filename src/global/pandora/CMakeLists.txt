# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 EICrecon authors

if(PandoraSDK_FOUND)
  get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

  # Create plugin and library targets
  plugin_add(${PLUGIN_NAME})

  # Glob sources and headers
  plugin_glob_all(${PLUGIN_NAME})

  # Add dependencies
  plugin_add_event_model(${PLUGIN_NAME})

  # Link libraries
  plugin_link_libraries(
    ${PLUGIN_NAME}
    algorithms_pandora_library
    PandoraPFA::PandoraSDK
  )
endif()
