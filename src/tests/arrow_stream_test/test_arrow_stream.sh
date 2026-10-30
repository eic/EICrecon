#!/bin/bash
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026, EICrecon contributors
#
# CI Test for Arrow IPC Stream Reader
#
# This test is only added when EICRECON_HAS_ARROW is enabled. It verifies that
# the installed EICrecon contains the Arrow stream reader:
# 1. The installed eicrecon executable exists
# 2. The installed podio plugin exists
# 3. The podio plugin contains the JEventSourcePODIOArrowStream event source
#
# It does not yet stream events from npsim/ddsim into eicrecon.

set -euo pipefail

if [ -z "${EICRECON_INSTALL_PREFIX:-}" ]; then
    echo "Error: EICRECON_INSTALL_PREFIX is not set"
    exit 1
fi

EICRECON="${EICRECON_INSTALL_PREFIX}/bin/eicrecon"
PODIO_PLUGIN="${EICRECON_INSTALL_PREFIX}/lib/EICrecon/plugins/podio.so"

if [ ! -x "${EICRECON}" ]; then
    echo "Error: eicrecon not found at ${EICRECON}"
    exit 1
fi
echo "✓ eicrecon found: ${EICRECON}"

if [ ! -f "${PODIO_PLUGIN}" ]; then
    echo "Error: podio plugin not found at ${PODIO_PLUGIN}"
    exit 1
fi
echo "✓ podio plugin found: ${PODIO_PLUGIN}"

# Prefer the demangled dynamic symbol table; fall back to searching the binary
if command -v nm &> /dev/null; then
    # Capture first: with pipefail, nm | grep -q can fail when nm gets SIGPIPE
    SYMBOLS=$(nm -D -C "${PODIO_PLUGIN}")
    if ! grep -q "JEventSourcePODIOArrowStream::" <<< "${SYMBOLS}"; then
        echo "Error: JEventSourcePODIOArrowStream symbols not found in ${PODIO_PLUGIN}"
        exit 1
    fi
elif ! grep -q -a "JEventSourcePODIOArrowStream" "${PODIO_PLUGIN}"; then
    echo "Error: JEventSourcePODIOArrowStream not found in ${PODIO_PLUGIN}"
    exit 1
fi
echo "✓ JEventSourcePODIOArrowStream found in podio plugin"
