# SPDX-License-Identifier: LGPL-3.0-or-later
"""DIRC wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import digi, meta

PLUGIN = "DIRC"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(meta.SubDivideCollection_SimTrackerHit_true, 'DIRCSurfaceHits',
                ['DIRCHits'],
                ['DIRCMCPHits', 'DIRCQuartzHits'],
                function=meta.GeometrySplit_SimTrackerHit([[0], [1]], "DIRCHits", ["surface"]))
        app.add(meta.CollectionCollector_SimTrackerHit_true, 'DIRCPhotonHits',
                ['DIRCBarHits', 'DIRCMCPHits'],
                ['DIRCPhotonHits'])
        app.add(digi.PhotoMultiplierHitDigi, 'DIRCRawHits',
                ['EventHeader', 'DIRCPhotonHits'],
                ['DIRCRawHits', 'DIRCRawHitsLinks', 'DIRCRawHitsAssociations'],
                enableQuantumEfficiency=False,
                quantumEfficiency=[])
