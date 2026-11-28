# SPDX-License-Identifier: LGPL-3.0-or-later
"""BTRK wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import digi, meta, tracking

PLUGIN = "BTRK"


def configure(app, plugin=PLUGIN):
    timeResolution = 2 * eu.us

    with app.plugin(plugin):
        app.add(digi.SiliconTrackerDigi, 'SiBarrelRawHits',
                ['EventHeader', 'SiBarrelHits'],
                ['SiBarrelRawHits', 'SiBarrelRawHitLinks', 'SiBarrelRawHitAssociations'],
                threshold=0.54 * u.keV,
                timeResolution=timeResolution)
        app.add(digi.RandomNoisePixel, 'SiBarrelNoiseRawHits',
                ['EventHeader'],
                ['SiBarrelNoiseRawHits'],
                addNoise=True,
                readout_name="SiBarrelHits")
        app.add(meta.CollectionCollector_RawTrackerHit, 'SiBarrelRawHitsWithNoise',
                ['SiBarrelRawHits', 'SiBarrelNoiseRawHits'],
                ['SiBarrelRawHitsWithNoise'])
        app.add(tracking.TrackerHitReconstruction, 'SiBarrelTrackerRecHits',
                ['SiBarrelRawHitsWithNoise'],
                ['SiBarrelTrackerRecHits'],
                timeResolution=timeResolution)
