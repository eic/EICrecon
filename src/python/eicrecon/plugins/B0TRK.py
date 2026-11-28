# SPDX-License-Identifier: LGPL-3.0-or-later
"""B0TRK wiring, converted from src/detectors/B0TRK/B0TRK.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "B0TRK"


def configure(app, plugin=PLUGIN):
    T = bindings("B0TRK")
    with app.plugin(plugin):
        app.add(T.SiliconTrackerDigi, 'B0TrackerRawHits',
                ['EventHeader', 'B0TrackerHits'],
                ['B0TrackerRawHits', 'B0TrackerRawHitLinks', 'B0TrackerRawHitAssociations'],
                threshold=5.0 * u.keV,
                timeResolution=30 * eu.ps)
        app.add(T.TrackerHitReconstruction, 'B0TrackerRecHits',
                ['B0TrackerRawHits'],
                ['B0TrackerRecHits'],
                timeResolution=30 * eu.ps)
