# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Factory types, one submodule per algorithm domain.

Importing a submodule loads only the C++ algorithm library it needs, e.g.
``eicrecon.factories.calorimetry`` loads libalgorithms_calorimetry.so.
"""
