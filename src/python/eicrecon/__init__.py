# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""EICrecon Python frontend.

Defines EICrecon algorithm graphs in Python and runs them with JANA2. The algorithms and
JOmniFactory classes stay in C++; Python only wires them. See docs/design/python_frontend.md.
"""

import os
import sys

# JANA plugins are dlopen'ed later and must resolve symbols (JANA, podio, EDM types) against the
# libraries already loaded by the extension modules; load ours with RTLD_GLOBAL as ROOT/Gaudi do.
_dlflags = sys.getdlopenflags()
sys.setdlopenflags(_dlflags | os.RTLD_GLOBAL)
try:
    from . import _core  # noqa: F401
finally:
    sys.setdlopenflags(_dlflags)

from .application import Application, GraphError  # noqa: E402,F401
from . import units  # noqa: E402,F401
