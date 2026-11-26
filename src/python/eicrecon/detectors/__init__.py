# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Python graph definitions that replace C++ detector plugins.

Each module defines ``configure(app)`` and is registered in ``PORTED`` under the name of the
C++ plugin it replaces, so the default graph can load either one (side-by-side migration).
"""

PORTED = {
    "B0ECAL": "eicrecon.detectors.b0ecal",
}
