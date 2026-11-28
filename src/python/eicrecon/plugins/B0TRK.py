# SPDX-License-Identifier: LGPL-3.0-or-later
"""B0TRK wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import digi, tracking

PLUGIN = "B0TRK"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(digi.SiliconTrackerDigi, 'B0TrackerRawHits',
                ['EventHeader', 'B0TrackerHits'],
                ['B0TrackerRawHits', 'B0TrackerRawHitLinks', 'B0TrackerRawHitAssociations'],
                threshold=5.0 * u.keV,
                timeResolution=30 * eu.ps)
        app.add(tracking.TrackerHitReconstruction, 'B0TrackerRecHits',
                ['B0TrackerRawHits'],
                ['B0TrackerRecHits'],
                timeResolution=30 * eu.ps)
