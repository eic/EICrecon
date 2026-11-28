# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""PID lookup table factory types (src/factories/pid_lut), bound by eicrecon._pid_lut."""

from . import _load

globals().update(_load("pid_lut"))
