# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Digitization factory types (src/factories/digi), bound by eicrecon._digi."""

from . import _load

globals().update(_load("digi"))
