# SPDX-License-Identifier: LGPL-3.0-or-later
"""DIRC wiring, converted from src/detectors/DIRC/DIRC.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "DIRC"


def configure(app, plugin=PLUGIN):
    T = bindings("DIRC")
    with app.plugin(plugin):
        app.add(T.SubDivideCollection_SimTrackerHit_true, 'DIRCSurfaceHits',
                ['DIRCHits'],
                ['DIRCMCPHits', 'DIRCQuartzHits'],
                function=T.GeometrySplit_SimTrackerHit([[0], [1]], "DIRCHits", ["surface"]))
        app.add(T.CollectionCollector_SimTrackerHit_true, 'DIRCPhotonHits',
                ['DIRCBarHits', 'DIRCMCPHits'],
                ['DIRCPhotonHits'])
        app.add(T.PhotoMultiplierHitDigi, 'DIRCRawHits',
                ['EventHeader', 'DIRCPhotonHits'],
                ['DIRCRawHits', 'DIRCRawHitsLinks', 'DIRCRawHitsAssociations'],
                enableQuantumEfficiency=False,
                quantumEfficiency=[])
