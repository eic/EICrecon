# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Calorimetry factory types (src/factories/calorimetry), bound by eicrecon._calorimetry."""

from . import _load

globals().update(_load("calorimetry"))
