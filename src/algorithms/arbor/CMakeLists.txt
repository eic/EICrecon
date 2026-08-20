# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 EICrecon authors

# Arbor settings installation
# Note: Arbor algorithms are provided by the ArborPFA package; this directory
# only contains the settings file for EIC detector configuration.

# Install Arbor settings XML alongside the library
install(FILES data/ArborPFASettings.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/arbor
)
