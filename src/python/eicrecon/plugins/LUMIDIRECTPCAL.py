# SPDX-License-Identifier: LGPL-3.0-or-later
"""LUMIDIRECTPCAL wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "LUMIDIRECTPCAL"


def configure(app, plugin=PLUGIN):
    T = bindings("LUMIDIRECTPCAL")
    with app.plugin(plugin):
        app.add(T.CalorimeterHitDigi, 'LumiDirectPCALRawHits',
                ['EventHeader', 'LumiDirectPCALHits'],
                ['LumiDirectPCALRawHits', 'LumiDirectPCALRawHitLinks', 'LumiDirectPCALRawHitAssociations'],
                eRes=[0.0 * sqrt(u.GeV), 0.02, 0.0 * u.GeV],
                capADC=16384,
                dyRangeADC=20 * u.GeV,
                pedMeanADC=100,
                pedSigmaADC=1,
                resolutionTDC=10 * u.picosecond,
                readout="LumiDirectPCALHits")
