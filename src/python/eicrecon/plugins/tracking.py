# SPDX-License-Identifier: LGPL-3.0-or-later
"""tracking wiring, converted from src/global/tracking/tracking.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "tracking"


def configure(app, plugin=PLUGIN):
    T = bindings("tracking")
    with app.plugin(plugin):
        app.add(T.TrackParamTruthInit, 'CentralTrackerTruthSeeds',
                ['EventHeader', 'MCParticles'],
                ['CentralTrackerTruthSeeds', 'CentralTrackerTruthSeedParameters'])
        app.add(T.TrackParamTruthInit, 'B0TrackerTruthSeeds',
                ['EventHeader', 'MCParticles'],
                ['B0TrackerTruthSeeds', 'B0TrackerTruthSeedParameters'],
                maxVertexX=24.0,
                maxVertexY=24.0,
                maxVertexZ=600.0,
                maxEtaBackward=0.0,
                useVertexAsPerigee=True)
        app.add(T.CollectionCollector_TrackerHit_true, 'CentralTrackingRecHits',
                ['SiBarrelTrackerRecHits', 'SiBarrelVertexRecHits', 'SiEndcapTrackerRecHits', 'MPGDBarrelRecHits', 'OuterMPGDBarrelRecHits', 'BackwardMPGDEndcapRecHits', 'ForwardMPGDEndcapRecHits'],
                ['CentralTrackingRecHits'])
        app.add(T.CollectionCollector_MCRecoTrackerHitAssociation_true, 'CentralTrackingRawHitAssociations',
                ['SiBarrelRawHitAssociations', 'SiBarrelVertexRawHitAssociations', 'SiEndcapTrackerRawHitAssociations', 'MPGDBarrelRawHitAssociations', 'OuterMPGDBarrelRawHitAssociations', 'BackwardMPGDEndcapRawHitAssociations', 'ForwardMPGDEndcapRawHitAssociations', 'TOFBarrelSharedRawHitAssociations', 'TOFEndcapSharedRawHitAssociations'],
                ['CentralTrackingRawHitAssociations'])
        app.add(T.CollectionCollector_MCRecoTrackerHitLink_true, 'CentralTrackingRawHitLinks',
                ['SiBarrelRawHitLinks', 'SiBarrelVertexRawHitLinks', 'SiEndcapTrackerRawHitLinks', 'TOFBarrelSharedRawHitLinks', 'TOFEndcapSharedRawHitLinks', 'MPGDBarrelRawHitLinks', 'OuterMPGDBarrelRawHitLinks', 'BackwardMPGDEndcapRawHitLinks', 'ForwardMPGDEndcapRawHitLinks'],
                ['CentralTrackingRawHitLinks'])
        app.add(T.TrackerMeasurementFromHits, 'CentralWithoutTOFTrackerMeasurements',
                ['CentralTrackingRecHits'],
                ['CentralWithoutTOFTrackerMeasurements'])
        app.add(T.CollectionCollector_Measurement2D_true, 'CentralTrackerMeasurements',
                ['CentralWithoutTOFTrackerMeasurements', 'TOFBarrelClusterHits', 'TOFEndcapClusterHits'],
                ['CentralTrackerMeasurements'])
        app.add(T.CKFTracking, 'CentralCKFTruthSeededTrajectories',
                ['CentralTrackerTruthSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFTruthSeededActsTrackStatesUnfiltered', 'CentralCKFTruthSeededActsTracksUnfiltered'])
        app.add(T.ActsToTracks, 'CentralCKFTruthSeededTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededActsTrackStatesUnfiltered', 'CentralCKFTruthSeededActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTrajectoriesUnfiltered', 'CentralCKFTruthSeededTrackParametersUnfiltered', 'CentralCKFTruthSeededTracksUnfiltered', 'CentralCKFTruthSeededTrackUnfilteredLinks', 'CentralCKFTruthSeededTrackUnfilteredAssociations'])
        app.add(T.CKFTracking, 'CentralCKFTruthSeededTimeTrajectories',
                ['CentralTrackerTruthSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFTruthSeededTimeActsTrackStatesUnfiltered', 'CentralCKFTruthSeededTimeActsTracksUnfiltered'],
                timeReadouts=['MPGDBarrelHits', 'OuterMPGDBarrelHits', 'BackwardMPGDEndcapHits', 'ForwardMPGDEndcapHits', 'TOFBarrelHits', 'TOFEndcapHits'])
        app.add(T.ActsToTracks, 'CentralCKFTruthSeededTimeTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededTimeActsTrackStatesUnfiltered', 'CentralCKFTruthSeededTimeActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTimeTrajectoriesUnfiltered', 'CentralCKFTruthSeededTimeTrackParametersUnfiltered', 'CentralCKFTruthSeededTimeTracksUnfiltered', 'CentralCKFTruthSeededTimeTrackUnfilteredLinks', 'CentralCKFTruthSeededTimeTrackUnfilteredAssociations'])
        app.add(T.AmbiguitySolver, 'CentralCKFTruthSeededTimeAmbiguityResolutionSolver',
                ['CentralCKFTruthSeededTimeActsTrackStatesUnfiltered', 'CentralCKFTruthSeededTimeActsTracksUnfiltered'],
                ['CentralCKFTruthSeededTimeActsTrackStates', 'CentralCKFTruthSeededTimeActsTracks'])
        app.add(T.ActsToTracks, 'CentralCKFTruthSeededTimeTracks',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededTimeActsTrackStates', 'CentralCKFTruthSeededTimeActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTimeTrajectories', 'CentralCKFTruthSeededTimeTrackParameters', 'CentralCKFTruthSeededTimeTracks', 'CentralCKFTruthSeededTimeTrackLinks', 'CentralCKFTruthSeededTimeTrackAssociations'])
        app.add(T.AmbiguitySolver, 'TruthSeededAmbiguityResolutionSolver',
                ['CentralCKFTruthSeededActsTrackStatesUnfiltered', 'CentralCKFTruthSeededActsTracksUnfiltered'],
                ['CentralCKFTruthSeededActsTrackStates', 'CentralCKFTruthSeededActsTracks'])
        app.add(T.ActsToTracks, 'CentralCKFTruthSeededTracks',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededActsTrackStates', 'CentralCKFTruthSeededActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTrajectories', 'CentralCKFTruthSeededTrackParameters', 'CentralCKFTruthSeededTracks', 'CentralCKFTruthSeededTrackLinks', 'CentralCKFTruthSeededTrackAssociations'])
        app.add(T.TrackSeeding, 'CentralTrackSeeds',
                ['CentralTrackingRecHits'],
                ['CentralTrackSeeds', 'CentralTrackSeedParameters'])
        app.add(T.CKFTracking, 'CentralCKFTrajectories',
                ['CentralTrackSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFActsTrackStatesUnfiltered', 'CentralCKFActsTracksUnfiltered'])
        app.add(T.ActsToTracks, 'CentralCKFTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFActsTrackStatesUnfiltered', 'CentralCKFActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTrajectoriesUnfiltered', 'CentralCKFTrackParametersUnfiltered', 'CentralCKFTracksUnfiltered', 'CentralCKFTrackUnfilteredLinks', 'CentralCKFTrackUnfilteredAssociations'])
        app.add(T.CKFTracking, 'CentralCKFTimeTrajectories',
                ['CentralTrackSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFTimeActsTrackStatesUnfiltered', 'CentralCKFTimeActsTracksUnfiltered'],
                timeReadouts=['MPGDBarrelHits', 'OuterMPGDBarrelHits', 'BackwardMPGDEndcapHits', 'ForwardMPGDEndcapHits', 'TOFBarrelHits', 'TOFEndcapHits'])
        app.add(T.ActsToTracks, 'CentralCKFTimeTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFTimeActsTrackStatesUnfiltered', 'CentralCKFTimeActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTimeTrajectoriesUnfiltered', 'CentralCKFTimeTrackParametersUnfiltered', 'CentralCKFTimeTracksUnfiltered', 'CentralCKFTimeTrackUnfilteredLinks', 'CentralCKFTimeTrackUnfilteredAssociations'])
        app.add(T.AmbiguitySolver, 'CentralCKFTimeAmbiguityResolutionSolver',
                ['CentralCKFTimeActsTrackStatesUnfiltered', 'CentralCKFTimeActsTracksUnfiltered'],
                ['CentralCKFTimeActsTrackStates', 'CentralCKFTimeActsTracks'])
        app.add(T.ActsToTracks, 'CentralCKFTimeTracks',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFTimeActsTrackStates', 'CentralCKFTimeActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTimeTrajectories', 'CentralCKFTimeTrackParameters', 'CentralCKFTimeTracks', 'CentralCKFTimeTrackLinks', 'CentralCKFTimeTrackAssociations'])
        app.add(T.AmbiguitySolver, 'AmbiguityResolutionSolver',
                ['CentralCKFActsTrackStatesUnfiltered', 'CentralCKFActsTracksUnfiltered'],
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks'])
        app.add(T.ActsToTracks, 'CentralCKFTracks',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTrajectories', 'CentralCKFTrackParameters', 'CentralCKFTracks', 'CentralCKFTrackLinks', 'CentralCKFTrackAssociations'])
        app.add(T.TrackProjector, 'CentralTrackSegments',
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'CentralCKFTracks'],
                ['CentralTrackSegments'])
        app.add(T.IterativeVertexFinder, 'CentralTrackVertices',
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'ReconstructedChargedParticles'],
                ['CentralTrackVertices'])
        app.add(T.TrackPropagation, 'CalorimeterTrackPropagator',
                ['CentralCKFTracks', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks'],
                ['CalorimeterTrackProjections'],
                target_surfaces=[{'type': 'CylinderSurfaceConfig', 'id': 'BarrelDIRC_ID', 'rmin': 'DIRC_rmin', 'zmin': 'DIRC_offset - DIRC_length / 2.0', 'zmax': 'DIRC_offset + DIRC_length / 2.0'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapN_ID', 'zmin': '- EcalEndcapN_zmin', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapN_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapN_ID', 'zmin': '- EcalEndcapN_zmin - 50*mm', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapN_rmax'}, {'type': 'CylinderSurfaceConfig', 'id': 'EcalBarrel_ID', 'rmin': 'EcalBarrel_rmin', 'zmin': '- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)', 'zmax': '1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)'}, {'type': 'CylinderSurfaceConfig', 'id': 'EcalBarrel_ID', 'rmin': 'EcalBarrel_rmin + 50*mm', 'zmin': '- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)', 'zmax': '1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapP_ID', 'zmin': 'EcalEndcapP_zmin', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapP_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapP_ID', 'zmin': 'EcalEndcapP_zmin + 50*mm', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapP_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'HcalEndcapN_ID', 'zmin': '- HcalEndcapN_zmin', 'rmin': 0.0, 'rmax': '1.1*HcalEndcapN_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'HcalEndcapN_ID', 'zmin': '- HcalEndcapN_zmin - 150*mm', 'rmin': 0.0, 'rmax': '1.1*HcalEndcapN_rmax'}, {'type': 'CylinderSurfaceConfig', 'id': 'HcalBarrel_ID', 'rmin': 'HcalBarrel_rmin', 'zmin': '- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)', 'zmax': '1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)'}, {'type': 'CylinderSurfaceConfig', 'id': 'HcalBarrel_ID', 'rmin': 'HcalBarrel_rmin + 150*mm', 'zmin': '- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)', 'zmax': '1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)'}, {'type': 'DiscSurfaceConfig', 'id': 'LFHCAL_ID', 'zmin': 'LFHCAL_zmin', 'rmin': 0.0, 'rmax': '1.1*LFHCAL_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'LFHCAL_ID', 'zmin': 'LFHCAL_zmin + 150*mm', 'rmin': 0.0, 'rmax': '1.1*LFHCAL_rmax'}])
        app.add(T.TrackerMeasurementFromHits, 'B0TrackerMeasurements',
                ['B0TrackerRecHits'],
                ['B0TrackerMeasurements'])
        app.add(T.CKFTracking, 'B0TrackerCKFTruthSeededTrajectories',
                ['B0TrackerTruthSeeds', 'B0TrackerMeasurements'],
                ['B0TrackerCKFTruthSeededActsTrackStatesUnfiltered', 'B0TrackerCKFTruthSeededActsTracksUnfiltered'],
                numMeasurementsMin=3)
        app.add(T.ActsToTracks, 'B0TrackerCKFTruthSeededTracksUnfiltered',
                ['B0TrackerMeasurements', 'B0TrackerTruthSeeds', 'B0TrackerCKFTruthSeededActsTrackStatesUnfiltered', 'B0TrackerCKFTruthSeededActsTracksUnfiltered', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTruthSeededTrajectoriesUnfiltered', 'B0TrackerCKFTruthSeededTrackParametersUnfiltered', 'B0TrackerCKFTruthSeededTracksUnfiltered', 'B0TrackerCKFTruthSeededTrackUnfilteredLinks', 'B0TrackerCKFTruthSeededTrackUnfilteredAssociations'])
        app.add(T.AmbiguitySolver, 'B0TrackerTruthSeededAmbiguityResolutionSolver',
                ['B0TrackerCKFTruthSeededActsTrackStatesUnfiltered', 'B0TrackerCKFTruthSeededActsTracksUnfiltered'],
                ['B0TrackerCKFTruthSeededActsTrackStates', 'B0TrackerCKFTruthSeededActsTracks'],
                n_measurements_min=3)
        app.add(T.ActsToTracks, 'B0TrackerCKFTruthSeededTracks',
                ['B0TrackerMeasurements', 'B0TrackerTruthSeeds', 'B0TrackerCKFTruthSeededActsTrackStates', 'B0TrackerCKFTruthSeededActsTracks', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTruthSeededTrajectories', 'B0TrackerCKFTruthSeededTrackParameters', 'B0TrackerCKFTruthSeededTracks', 'B0TrackerCKFTruthSeededTrackLinks', 'B0TrackerCKFTruthSeededTrackAssociations'])
        app.add(T.B0TrackSeeding, 'B0TrackerSeeds',
                ['B0TrackerRecHits'],
                ['B0TrackerSeeds', 'B0TrackerSeedParameters'])
        app.add(T.CKFTracking, 'B0TrackerCKFTrajectories',
                ['B0TrackerSeeds', 'B0TrackerMeasurements'],
                ['B0TrackerCKFActsTrackStatesUnfiltered', 'B0TrackerCKFActsTracksUnfiltered'],
                numMeasurementsMin=3)
        app.add(T.ActsToTracks, 'B0TrackerCKFTracksUnfiltered',
                ['B0TrackerMeasurements', 'B0TrackerSeeds', 'B0TrackerCKFActsTrackStatesUnfiltered', 'B0TrackerCKFActsTracksUnfiltered', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTrajectoriesUnfiltered', 'B0TrackerCKFTrackParametersUnfiltered', 'B0TrackerCKFTracksUnfiltered', 'B0TrackerCKFTrackUnfilteredLinks', 'B0TrackerCKFTrackUnfilteredAssociations'])
        app.add(T.AmbiguitySolver, 'B0TrackerAmbiguityResolutionSolver',
                ['B0TrackerCKFActsTrackStatesUnfiltered', 'B0TrackerCKFActsTracksUnfiltered'],
                ['B0TrackerCKFActsTrackStates', 'B0TrackerCKFActsTracks'],
                n_measurements_min=3)
        app.add(T.ActsToTracks, 'B0TrackerCKFTracks',
                ['B0TrackerMeasurements', 'B0TrackerSeeds', 'B0TrackerCKFActsTrackStates', 'B0TrackerCKFActsTracks', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTrajectories', 'B0TrackerCKFTrackParameters', 'B0TrackerCKFTracks', 'B0TrackerCKFTrackLinks', 'B0TrackerCKFTrackAssociations'])
        app.add(T.ActsTrackMerger, 'CentralAndB0TrackerCKFActsTracks',
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'B0TrackerCKFActsTrackStates', 'B0TrackerCKFActsTracks'],
                ['CentralAndB0TrackerCKFActsTrackStates', 'CentralAndB0TrackerCKFActsTracks'])
        app.add(T.IterativeVertexFinder, 'CentralAndB0TrackVertices',
                ['CentralAndB0TrackerCKFActsTrackStates', 'CentralAndB0TrackerCKFActsTracks', 'ReconstructedChargedParticles'],
                ['CentralAndB0TrackVertices'])
        app.add(T.CollectionCollector_Track_true, 'CombinedTracks',
                ['CentralCKFTracks', 'B0TrackerCKFTracks'],
                ['CombinedTracks'])
        app.add(T.SecondaryVertexFinder, 'PrimaryVerticesAMVF',
                ['ReconstructedParticles', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks'],
                ['PrimaryVerticesAMVF'])
        app.add(T.SecondaryVertexFinder, 'SecondaryVerticesAMVF',
                ['ReconstructedParticles', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks'],
                ['SecondaryVerticesAMVF'],
                isPrimary=False,
                maxIterations=1000,
                tracksMaxZinterval=1000.0,
                useSeedConstraint=True,
                useTwoTrackSeeder=True)
        app.add(T.CollectionCollector_MCRecoTrackParticleAssociation_true, 'CombinedTrackAssociations',
                ['CentralCKFTrackAssociations', 'B0TrackerCKFTrackAssociations'],
                ['CombinedTrackAssociations'])
        app.add(T.CollectionCollector_Track_true, 'CombinedTruthSeededTracks',
                ['CentralCKFTruthSeededTracks', 'B0TrackerCKFTruthSeededTracks'],
                ['CombinedTruthSeededTracks'])
        app.add(T.CollectionCollector_MCRecoTrackParticleAssociation_true, 'CombinedTruthSeededTrackAssociations',
                ['CentralCKFTruthSeededTrackAssociations', 'B0TrackerCKFTruthSeededTrackAssociations'],
                ['CombinedTruthSeededTrackAssociations'])
        app.add(T.CollectionCollector_MCRecoTrackParticleLink_true, 'CombinedTruthSeededTrackLinks',
                ['CentralCKFTruthSeededTrackLinks', 'B0TrackerCKFTruthSeededTrackLinks'],
                ['CombinedTruthSeededTrackLinks'])
        app.add(T.TracksToParticles, 'ChargedTruthSeededParticlesWithAssociations',
                ['CombinedTruthSeededTracks', 'CombinedTruthSeededTrackAssociations'],
                ['ReconstructedTruthSeededChargedWithoutPIDParticles', 'ReconstructedTruthSeededChargedWithoutPIDParticleLinks', 'ReconstructedTruthSeededChargedWithoutPIDParticleAssociations'])
        app.add(T.TracksToParticles, 'ChargedParticlesWithAssociations',
                ['CombinedTracks', 'CombinedTrackAssociations'],
                ['ReconstructedChargedWithoutPIDParticles', 'ReconstructedChargedWithoutPIDParticleLinks', 'ReconstructedChargedWithoutPIDParticleAssociations'])
