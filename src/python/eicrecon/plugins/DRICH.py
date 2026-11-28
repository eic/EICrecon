# SPDX-License-Identifier: LGPL-3.0-or-later
"""DRICH wiring, ported by hand from the former C++ InitPlugin.

The track propagation surfaces and track point cuts of the two radiators come from the RichGeo
service (geometry), through pid.track_propagation_configs in a deferred section.

Values use dd4hep units (u, as in the C++ wiring).
"""

from ..units import dd4hep as u
from ..factories import digi, pid

PLUGIN = "DRICH"


def configure(app, plugin=PLUGIN):

    # digitization
    digi_cfg = dict(
        detectorName="DRICH",
        readoutClass="DRICHHits",
        hitTimeWindow=20.0,  # [ns]
        timeResolution=1 / 16.0,  # [ns]
        speMean=80.0,
        speError=16.0,
        pedMean=200.0,
        pedError=3.0,
        enablePixelGaps=True,
        safetyFactor=0.7,
        enableNoise=False,
        noiseRate=20000,  # [Hz]
        noiseTimeWindow=20.0 * u.ns,  # [ns]
        quantumEfficiency=[  # wavelength units are [nm]
            [315, 0.00], [325, 0.04], [340, 0.10], [350, 0.20], [370, 0.30], [400, 0.35],
            [450, 0.40], [500, 0.38], [550, 0.35], [600, 0.27], [650, 0.20], [700, 0.15],
            [750, 0.12], [800, 0.08], [850, 0.06], [900, 0.04], [1000, 0.00]],
    )

    # IRT PID
    irt_cfg = dict(
        numRIndexBins=100,  # refractive index interpolation
        radiators={
            "Aerogel": dict(referenceRIndex=1.0260, attenuation=48,  # [mm]
                            smearingMode="gaussian", smearing=2e-3),  # [radians]
            "Gas": dict(referenceRIndex=1.00076, attenuation=0,  # [mm]
                        smearingMode="gaussian", smearing=5e-3),  # [radians]
        },
        pdgList=[11, 211, 321, 2212],
        cheatPhotonVertex=False,
        cheatTrueRadiator=False,
    )

    tracks_in = ["CentralCKFTracks", "CentralCKFActsTrackStates", "CentralCKFActsTracks"]

    def track_propagation(graph):
        # surfaces and cuts from the RichGeo service (empty if DRICH is not in the geometry)
        cfgs = pid.track_propagation_configs(graph.ctx.app)
        graph.add(pid.RichTrack, "DRICHAerogelTracks", tracks_in, ["DRICHAerogelTracks"],
                  **cfgs["aerogel"])
        graph.add(pid.RichTrack, "DRICHGasTracks", tracks_in, ["DRICHGasTracks"], **cfgs["gas"])

    with app.plugin(plugin):
        app.add(digi.PhotoMultiplierHitDigi, "DRICHRawHits",
                ["EventHeader", "DRICHHits"],
                ["DRICHRawHits", "DRICHRawHitsLinks", "DRICHRawHitsAssociations"],
                **digi_cfg)
        app.deferred(track_propagation)
        app.add(pid.MergeTrack, "DRICHMergedTracks",
                ["DRICHAerogelTracks", "DRICHGasTracks"],
                ["DRICHMergedTracks"])
        app.add(pid.IrtCherenkovParticleID, "DRICHIrtCherenkovParticleID",
                ["DRICHAerogelTracks", "DRICHGasTracks", "DRICHMergedTracks", "DRICHRawHits",
                 "DRICHRawHitsLinks", "DRICHRawHitsAssociations"],
                ["DRICHAerogelIrtCherenkovParticleID", "DRICHGasIrtCherenkovParticleID"],
                **irt_cfg)
        app.add(pid.MergeCherenkovParticleID, "DRICHMergedIrtCherenkovParticleID",
                ["DRICHAerogelIrtCherenkovParticleID", "DRICHGasIrtCherenkovParticleID"],
                ["DRICHMergedIrtCherenkovParticleID"],
                mergeMode=0)  # MergeParticleIDConfig::kAddWeights
