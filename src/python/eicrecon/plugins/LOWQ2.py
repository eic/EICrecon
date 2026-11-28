# SPDX-License-Identifier: LGPL-3.0-or-later
"""LOWQ2 wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import digi, fardetectors, meta, tracking

PLUGIN = "LOWQ2"


def configure(app, plugin=PLUGIN):
    readout = "TaggerTrackerHits"
    timeResolution = 2 * eu.ns

    with app.plugin(plugin):
        app.add(digi.SiliconChargeSharing, 'TaggerTrackerChargeSharing',
                ['TaggerTrackerHits'],
                ['TaggerTrackerSharedHits'],
                sigma_sharingx=15 * u.um,
                sigma_sharingy=15 * u.um,
                min_edep=0.1 * eu.keV,
                readout=readout)
        app.add(digi.PulseGeneration_SimTrackerHit, 'TaggerTrackerPulseGeneration',
                ['TaggerTrackerSharedHits'],
                ['TaggerTrackerHitPulses'],
                pulse_shape_params=[1.0, 2 * eu.ns],
                ignore_thres=15.0e-8)
        app.add(digi.PulseCombiner, 'TaggerTrackerPulseCombiner',
                ['TaggerTrackerHitPulses'],
                ['TaggerTrackerCombinedPulses'],
                minimum_separation=25 * eu.ns)
        app.add(digi.PulseNoise, 'TaggerTrackerPulseNoise',
                ['EventHeader', 'TaggerTrackerCombinedPulses'],
                ['TaggerTrackerCombinedPulsesWithNoise'],
                scale=0.000002)
        app.add(digi.SiliconTrackerDigi, 'TaggerTrackerRawHits',
                ['EventHeader', 'TaggerTrackerHits'],
                ['TaggerTrackerRawHits', 'TaggerTrackerRawHitLinks', 'TaggerTrackerRawHitAssociations'],
                threshold=1.5 * eu.keV,
                timeResolution=timeResolution)
        app.add(tracking.TrackerHitReconstruction, 'TaggerTrackerRecHits',
                ['TaggerTrackerRawHits'],
                ['TaggerTrackerRecHits'],
                timeResolution=timeResolution)
        app.add(meta.SubDivideCollection_TrackerHit, 'TaggerTrackerSplitHits',
                ['TaggerTrackerRecHits'],
                ['TaggerTrackerM1L0RecHits', 'TaggerTrackerM1L1RecHits', 'TaggerTrackerM1L2RecHits', 'TaggerTrackerM1L3RecHits', 'TaggerTrackerM2L0RecHits', 'TaggerTrackerM2L1RecHits', 'TaggerTrackerM2L2RecHits', 'TaggerTrackerM2L3RecHits'],
                function=meta.GeometrySplit_TrackerHit([[m, l] for m in [1, 2] for l in [0, 1, 2, 3]], "TaggerTrackerHits", ["module", "layer"]))
        app.add(fardetectors.FarDetectorTrackerCluster, 'TaggerTrackerClustering',
                ['TaggerTrackerM1L0RecHits', 'TaggerTrackerM1L1RecHits', 'TaggerTrackerM1L2RecHits', 'TaggerTrackerM1L3RecHits', 'TaggerTrackerM2L0RecHits', 'TaggerTrackerM2L1RecHits', 'TaggerTrackerM2L2RecHits', 'TaggerTrackerM2L3RecHits'],
                ['TaggerTrackerM1L0ClusterPositions', 'TaggerTrackerM1L1ClusterPositions', 'TaggerTrackerM1L2ClusterPositions', 'TaggerTrackerM1L3ClusterPositions', 'TaggerTrackerM2L0ClusterPositions', 'TaggerTrackerM2L1ClusterPositions', 'TaggerTrackerM2L2ClusterPositions', 'TaggerTrackerM2L3ClusterPositions'],
                readout="TaggerTrackerHits")
        app.add(meta.CollectionCollector_Track_true, 'TaggerTrackerLocalTracks',
                ['TaggerTrackerM1LocalTracks', 'TaggerTrackerM2LocalTracks'],
                ['TaggerTrackerLocalTracks'])
        app.add(meta.CollectionCollector_MCRecoTrackParticleLink_true, 'TaggerTrackerLocalTrackLinks',
                ['TaggerTrackerM1LocalTrackLinks', 'TaggerTrackerM2LocalTrackLinks'],
                ['TaggerTrackerLocalTrackLinks'])
        app.add(meta.CollectionCollector_MCRecoTrackParticleAssociation_true, 'TaggerTrackerLocalTrackAssociations',
                ['TaggerTrackerM1LocalTrackAssociations', 'TaggerTrackerM2LocalTrackAssociations'],
                ['TaggerTrackerLocalTrackAssociations'])
        app.add(fardetectors.FarDetectorTransportationPreML, 'TaggerTrackerTransportationPreML',
                ['TaggerTrackerLocalTracks', 'TaggerTrackerLocalTrackLinks', 'MCBeamElectrons'],
                ['TaggerTrackerFeatureTensor', 'TaggerTrackerTargetTensor'])
        app.add(meta.ONNXInference, 'TaggerTrackerTransportationInference',
                ['TaggerTrackerFeatureTensor'],
                ['TaggerTrackerPredictionTensor'],
                modelPath="calibrations/onnx/Low-Q2_Steering_Reconstruction.onnx")
        app.add(fardetectors.FarDetectorTransportationPostML, 'TaggerTrackerTransportationPostML',
                ['TaggerTrackerPredictionTensor', 'TaggerTrackerLocalTrackAssociations', 'MCBeamElectrons'],
                ['TaggerTrackerReconstructedParticles', 'TaggerTrackerReconstructedParticleLinks', 'TaggerTrackerReconstructedParticleAssociations'])
        app.add(fardetectors.FarDetectorLinearTracking, 'TaggerTrackerM1LocalTracks',
                ['TaggerTrackerM1L0ClusterPositions', 'TaggerTrackerM1L1ClusterPositions', 'TaggerTrackerM1L2ClusterPositions', 'TaggerTrackerM1L3ClusterPositions', 'TaggerTrackerRawHitLinks', 'TaggerTrackerRawHitAssociations'],
                ['TaggerTrackerM1LocalTracks', 'TaggerTrackerM1LocalTrackLinks', 'TaggerTrackerM1LocalTrackAssociations'],
                layer_hits_max=200,
                optimum_theta=-3.1155927181243896)
        app.add(fardetectors.FarDetectorLinearTracking, 'TaggerTrackerM2LocalTracks',
                ['TaggerTrackerM2L0ClusterPositions', 'TaggerTrackerM2L1ClusterPositions', 'TaggerTrackerM2L2ClusterPositions', 'TaggerTrackerM2L3ClusterPositions', 'TaggerTrackerRawHitLinks', 'TaggerTrackerRawHitAssociations'],
                ['TaggerTrackerM2LocalTracks', 'TaggerTrackerM2LocalTrackLinks', 'TaggerTrackerM2LocalTrackAssociations'],
                layer_hits_max=200,
                optimum_theta=-3.1155927181243896)
