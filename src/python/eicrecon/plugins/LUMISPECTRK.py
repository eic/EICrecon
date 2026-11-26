# SPDX-License-Identifier: LGPL-3.0-or-later
"""LUMISPECTRK wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "LUMISPECTRK"


def configure(app, plugin=PLUGIN):
    T = bindings("LUMISPECTRK")
    with app.plugin(plugin):
        app.add(T.SiliconTrackerDigi, 'LumiSpecTrackerRawHits',
                ['EventHeader', 'LumiSpecTrackerHits'],
                ['LumiSpecTrackerRawHits', 'LumiSpecTrackerRawHitLinks', 'LumiSpecTrackerRawHitAssociations'],
                threshold=10.0 * u.keV,
                timeResolution=30 * eu.ps)
