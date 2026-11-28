# SPDX-License-Identifier: LGPL-3.0-or-later
"""RPOTS wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import digi, fardetectors, reco, tracking

PLUGIN = "RPOTS"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(digi.SiliconTrackerDigi, 'ForwardRomanPotRawHits',
                ['EventHeader', 'ForwardRomanPotHits'],
                ['ForwardRomanPotRawHits', 'ForwardRomanPotRawHitLinks', 'ForwardRomanPotRawHitAssociations'],
                threshold=10.0 * u.keV,
                timeResolution=30 * eu.ps)
        app.add(tracking.TrackerHitReconstruction, 'ForwardRomanPotRecHits',
                ['ForwardRomanPotRawHits'],
                ['ForwardRomanPotRecHits'],
                timeResolution=30 * eu.ps)
        app.add(fardetectors.MatrixTransferStatic, 'ForwardRomanPotStaticRecParticles',
                ['MCParticles', 'ForwardRomanPotRecHits'],
                ['ForwardRomanPotStaticRecParticles'],
                matrix_configs=[{'nomMomentum': 275.0, 'aX': [[3.251116, 30.285734], [0.186036375, 0.196439472]], 'aY': [[0.47305, 3.062999454], [0.0204108951, -0.139318692]], 'local_x_offset': -1209.29, 'local_y_offset': 0.00132511, 'local_x_slope_offset': -45.4772, 'local_y_slope_offset': 0.000745498}, {'nomMomentum': 130.0, 'aX': [[3.16912, 22.4693], [0.182402, -0.218209]], 'aY': [[0.520743, 3.17339], [0.0222482, -0.0923779]], 'local_x_offset': -1209.29, 'local_y_offset': 0.00132511, 'local_x_slope_offset': -45.4772, 'local_y_slope_offset': 0.000745498}, {'nomMomentum': 100.0, 'aX': [[3.152158, 20.852072], [0.181649517, -0.303998487]], 'aY': [[0.53061, 3.19623343], [0.022628332, -0.082666019]], 'local_x_offset': -1209.27, 'local_y_offset': 0.00355218, 'local_x_slope_offset': -45.4737, 'local_y_slope_offset': 0.00204394}, {'nomMomentum': 41.0, 'aX': [[3.135997, 18.482273], [0.176479921, -0.497839483]], 'aY': [[0.49144, 4.53857451], [0.0179664765, 0.004160679]], 'local_x_offset': -1209.22, 'local_y_offset': 0.00868737, 'local_x_slope_offset': -45.4641, 'local_y_slope_offset': 0.00498786}],
                hit1minZ=32541.0,
                hit1maxZ=32554.0,
                hit2minZ=34239.0,
                hit2maxZ=34252.0,
                readout="ForwardRomanPotRecHits")
        app.add(fardetectors.PolynomialMatrixReconstruction, 'ForwardRomanPotRecParticles',
                ['MCParticles', 'ForwardRomanPotRecHits'],
                ['ForwardRomanPotRecParticles'],
                poly_matrix_configs=[{'nomMomentum': 275.0}, {'nomMomentum': 130.0}, {'nomMomentum': 100.0}, {'nomMomentum': 41.0}],
                hit1minZ=32541.0,
                hit1maxZ=32554.0,
                hit2minZ=34239.0,
                hit2maxZ=34252.0,
                readout="ForwardRomanPotRecHits")
        app.add(reco.TransformLocalFFFrameToLabFrame, 'ForwardRomanPotRecLabFrameParticles',
                ['ForwardRomanPotRecParticles'],
                ['ForwardRomanPotRecLabFrameParticles'])
