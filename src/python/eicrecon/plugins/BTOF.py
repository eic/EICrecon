# SPDX-License-Identifier: LGPL-3.0-or-later
"""BTOF wiring, converted from src/detectors/BTOF/BTOF.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "BTOF"


def configure(app, plugin=PLUGIN):
    T = bindings("BTOF")
    timeResolution = 25 * eu.ps
    sigma_analog = 0.293951 * eu.ns
    adc_range = 256
    offset = 3
    risetime = 0.45 * eu.ns

    with app.plugin(plugin):
        app.add(T.LGADHitCalibration, 'TOFBarrelCalibratedHits',
                ['TOFBarrelADCTDC'],
                ['TOFBarrelCalibratedHits'])
        app.add(T.LGADHitClustering, 'TOFBarrelClusterHits',
                ['TOFBarrelSharedRecHits'],
                ['TOFBarrelClusterHits'],
                useAve=True)
        app.add(T.TrackerHitReconstruction, 'TOFBarrelSharedRecHits',
                ['TOFBarrelSharedRawHits'],
                ['TOFBarrelSharedRecHits'],
                timeResolution=timeResolution)
        app.add(T.SiliconChargeSharing, 'TOFBarrelSharedHits',
                ['TOFBarrelHits'],
                ['TOFBarrelSharedHits'],
                sigma_mode=1,
                sigma_sharingx=0.5,
                sigma_sharingy=0.5,
                min_edep=6.0 * eu.keV,
                readout="TOFBarrelHits")
        app.add(T.SiliconTrackerDigi, 'TOFBarrelSharedRawHits',
                ['EventHeader', 'TOFBarrelSharedHits'],
                ['TOFBarrelSharedRawHits', 'TOFBarrelSharedRawHitLinks', 'TOFBarrelSharedRawHitAssociations'],
                timeResolution=timeResolution)
        app.add(T.PulseGeneration_SimTrackerHit, 'LGADPulseGeneration',
                ['TOFBarrelSharedHits'],
                ['TOFBarrelSmoothPulses'],
                pulse_shape_params=[-1388488.0400364662, 0.293951, 3.0],
                ignore_thres=0.05 * adc_range,
                timestep=0.01 * eu.ns)
        app.add(T.PulseCombiner, 'TOFBarrelPulseCombiner',
                ['TOFBarrelSmoothPulses'],
                ['TOFBarrelCombinedPulses'],
                minimum_separation=25 * eu.ns)
        app.add(T.SiliconPulseDiscretization, 'TOFBarrelPulses',
                ['TOFBarrelCombinedPulses'],
                ['TOFBarrelPulses'],
                global_offset=-offset * sigma_analog + risetime)
        app.add(T.CFDROCDigitization, 'CFDROCDigitization',
                ['TOFBarrelPulses'],
                ['TOFBarrelADCTDC'])
