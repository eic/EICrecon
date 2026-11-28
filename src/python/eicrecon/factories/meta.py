# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Generic (meta) factory types (src/factories/meta), bound by eicrecon._meta."""

from . import _load

globals().update(_load("meta"))
