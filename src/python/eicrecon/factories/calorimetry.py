# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Calorimetry factory types (bound in C++ by eicrecon._calorimetry)."""

import os
import sys

_dlflags = sys.getdlopenflags()
sys.setdlopenflags(_dlflags | os.RTLD_GLOBAL)
try:
    from .._calorimetry import (  # noqa: F401
        CalorimeterClusterRecoCoG,
        CalorimeterClusterShape,
        CalorimeterHitDigi,
        CalorimeterHitReco,
        CalorimeterIslandCluster,
        CalorimeterTruthClustering,
    )
finally:
    sys.setdlopenflags(_dlflags)
