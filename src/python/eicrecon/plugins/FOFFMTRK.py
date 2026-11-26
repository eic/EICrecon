# SPDX-License-Identifier: LGPL-3.0-or-later
"""FOFFMTRK wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "FOFFMTRK"


def configure(app, plugin=PLUGIN):
    T = bindings("FOFFMTRK")
    with app.plugin(plugin):
        app.add(T.SiliconTrackerDigi, 'ForwardOffMTrackerRawHits',
                ['EventHeader', 'ForwardOffMTrackerHits'],
                ['ForwardOffMTrackerRawHits', 'ForwardOffMTrackerRawHitLinks', 'ForwardOffMTrackerRawHitAssociations'],
                threshold=10.0 * u.keV,
                timeResolution=30 * eu.ps)
        app.add(T.TrackerHitReconstruction, 'ForwardOffMTrackerRecHits',
                ['ForwardOffMTrackerRawHits'],
                ['ForwardOffMTrackerRecHits'],
                timeResolution=30 * eu.ps)
        app.add(T.MatrixTransferStatic, 'ForwardOffMRecParticles',
                ['MCParticles', 'ForwardOffMTrackerRecHits'],
                ['ForwardOffMRecParticles'],
                matrix_configs=[{'nomMomentum': 130.0, 'aX': [[2.08344, 5.37571], [0.188756, -2.90941]], 'aY': [[-0.977013, -35.7785], [-0.0812252, -2.86315]], 'local_x_offset': -1032.2, 'local_y_offset': 0.00462829, 'local_x_slope_offset': -59.7363, 'local_y_slope_offset': -0.0030213}],
                hit1minZ=25490.0,
                hit1maxZ=25512.0,
                hit2minZ=27012.0,
                hit2maxZ=27035.0,
                readout="ForwardOffMTrackerRecHits")
        app.add(T.TransformLocalFFFrameToLabFrame, 'ForwardOffMRecLabFrameParticles',
                ['ForwardOffMRecParticles'],
                ['ForwardOffMRecLabFrameParticles'])
