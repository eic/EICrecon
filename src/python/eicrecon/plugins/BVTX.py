# SPDX-License-Identifier: LGPL-3.0-or-later
"""BVTX wiring, converted from src/detectors/BVTX/BVTX.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "BVTX"


def configure(app, plugin=PLUGIN):
    T = bindings("BVTX")
    timeResolution = 2 * eu.us

    with app.plugin(plugin):
        app.add(T.SiliconTrackerDigi, 'SiBarrelVertexRawHits',
                ['EventHeader', 'VertexBarrelHits'],
                ['SiBarrelVertexRawHits', 'SiBarrelVertexRawHitLinks', 'SiBarrelVertexRawHitAssociations'],
                threshold=0.54 * u.keV,
                timeResolution=timeResolution)
        app.add(T.RandomNoisePixel, 'SiBarrelVertexNoiseRawHits',
                ['EventHeader'],
                ['SiBarrelVertexNoiseRawHits'],
                addNoise=True)
        app.add(T.CollectionCollector_RawTrackerHit, 'SiBarrelVertexRawHitsWithNoise',
                ['SiBarrelVertexRawHits', 'SiBarrelVertexNoiseRawHits'],
                ['SiBarrelVertexRawHitsWithNoise'])
        app.add(T.TrackerHitReconstruction, 'SiBarrelVertexRecHits',
                ['SiBarrelVertexRawHitsWithNoise'],
                ['SiBarrelVertexRecHits'],
                timeResolution=timeResolution)
