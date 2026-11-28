# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Far-forward detector factory types (src/factories/fardetectors), bound by eicrecon._fardetectors."""

from . import _load

globals().update(_load("fardetectors"))
