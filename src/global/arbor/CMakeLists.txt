# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 EICrecon authors

if(ArborPFA_FOUND)
  add_plugin(eicrecon_arbor_plugin
    SOURCES arbor.cc
    HEADERS
    LINK_LIBRARIES eicrecon::pandora_algorithms)
endif()
