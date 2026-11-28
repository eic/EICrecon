# SPDX-License-Identifier: LGPL-3.0-or-later
"""ECTOF wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import digi, tracking

PLUGIN = "ECTOF"


def configure(app, plugin=PLUGIN):
    timeResolution = 25 * eu.ps
    sigma_analog = 0.293951 * eu.ns
    adc_range = 256
    offset = 3
    risetime = 0.45 * eu.ns

    with app.plugin(plugin):
        app.add(tracking.LGADHitClustering, 'TOFEndcapClusterHits',
                ['TOFEndcapSharedRecHits'],
                ['TOFEndcapClusterHits'],
                readout="TOFEndcapHits",
                useAve=True)
        app.add(tracking.TrackerHitReconstruction, 'TOFEndcapSharedRecHits',
                ['TOFEndcapSharedRawHits'],
                ['TOFEndcapSharedRecHits'],
                timeResolution=timeResolution)
        app.add(digi.SiliconChargeSharing, 'TOFEndcapSharedHits',
                ['TOFEndcapHits'],
                ['TOFEndcapSharedHits'],
                sigma_mode=1,
                sigma_sharingx=0.5,
                sigma_sharingy=0.5,
                min_edep=6 * u.keV,
                readout="TOFEndcapHits")
        app.add(digi.SiliconTrackerDigi, 'TOFEndcapSharedRawHits',
                ['EventHeader', 'TOFEndcapSharedHits'],
                ['TOFEndcapSharedRawHits', 'TOFEndcapSharedRawHitLinks', 'TOFEndcapSharedRawHitAssociations'],
                timeResolution=timeResolution)
        app.add(digi.PulseGeneration_SimTrackerHit, 'TOFEndcapSmoothPulses',
                ['TOFEndcapSharedHits'],
                ['TOFEndcapSmoothPulses'],
                pulse_shape_params=[-1388488.0400364662, 0.293951, 3.0],
                ignore_thres=0.05 * adc_range,
                timestep=0.01 * eu.ns)
        app.add(digi.PulseCombiner, 'TOFEndcapCombinedPulses',
                ['TOFEndcapSmoothPulses'],
                ['TOFEndcapCombinedPulses'],
                minimum_separation=25 * eu.ns)
        app.add(digi.SiliconPulseDiscretization, 'TOFEndcapPulses',
                ['TOFEndcapCombinedPulses'],
                ['TOFEndcapPulses'],
                global_offset=-offset * sigma_analog + risetime)
        app.add(digi.EICROCDigitization, 'TOFEndcapADCTDC',
                ['TOFEndcapPulses'],
                ['TOFEndcapADCTDC'])
