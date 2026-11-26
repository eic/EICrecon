# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Unit systems, with values taken from the C++ headers.

``dd4hep`` (cm = GeV = 1) is what the existing C++ plugin wiring uses for config structs, so
graph definitions ported from C++ use ``dd4hep`` units for identical values. ``edm4eic``
(mm = GeV = 1) is the EDM convention. See "Unit System Policy" in AGENTS.md.
"""

from ._core import dd4hep_units as dd4hep  # noqa: F401
from ._core import edm4eic_units as edm4eic  # noqa: F401
