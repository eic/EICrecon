# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 EICrecon authors

if(PandoraSDK_FOUND)
  add_plugin(eicrecon_pandora_plugin
    SOURCES pandora.cc
    HEADERS
    LINK_LIBRARIES eicrecon::pandora_algorithms)
endif()
