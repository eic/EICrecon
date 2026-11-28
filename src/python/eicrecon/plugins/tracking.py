# SPDX-License-Identifier: LGPL-3.0-or-later
"""tracking wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import meta, tracking

PLUGIN = "tracking"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(tracking.TrackParamTruthInit, 'CentralTrackerTruthSeeds',
                ['EventHeader', 'MCParticles'],
                ['CentralTrackerTruthSeeds', 'CentralTrackerTruthSeedParameters'])
        app.add(tracking.TrackParamTruthInit, 'B0TrackerTruthSeeds',
                ['EventHeader', 'MCParticles'],
                ['B0TrackerTruthSeeds', 'B0TrackerTruthSeedParameters'],
                maxVertexX=24.0,
                maxVertexY=24.0,
                maxVertexZ=600.0,
                maxEtaBackward=0.0,
                useVertexAsPerigee=True)
        app.add(meta.CollectionCollector_TrackerHit_true, 'CentralTrackingRecHits',
                ['SiBarrelTrackerRecHits', 'SiBarrelVertexRecHits', 'SiEndcapTrackerRecHits', 'MPGDBarrelRecHits', 'OuterMPGDBarrelRecHits', 'BackwardMPGDEndcapRecHits', 'ForwardMPGDEndcapRecHits'],
                ['CentralTrackingRecHits'])
        app.add(meta.CollectionCollector_MCRecoTrackerHitAssociation_true, 'CentralTrackingRawHitAssociations',
                ['SiBarrelRawHitAssociations', 'SiBarrelVertexRawHitAssociations', 'SiEndcapTrackerRawHitAssociations', 'MPGDBarrelRawHitAssociations', 'OuterMPGDBarrelRawHitAssociations', 'BackwardMPGDEndcapRawHitAssociations', 'ForwardMPGDEndcapRawHitAssociations', 'TOFBarrelSharedRawHitAssociations', 'TOFEndcapSharedRawHitAssociations'],
                ['CentralTrackingRawHitAssociations'])
        app.add(meta.CollectionCollector_MCRecoTrackerHitLink_true, 'CentralTrackingRawHitLinks',
                ['SiBarrelRawHitLinks', 'SiBarrelVertexRawHitLinks', 'SiEndcapTrackerRawHitLinks', 'TOFBarrelSharedRawHitLinks', 'TOFEndcapSharedRawHitLinks', 'MPGDBarrelRawHitLinks', 'OuterMPGDBarrelRawHitLinks', 'BackwardMPGDEndcapRawHitLinks', 'ForwardMPGDEndcapRawHitLinks'],
                ['CentralTrackingRawHitLinks'])
        app.add(tracking.TrackerMeasurementFromHits, 'CentralWithoutTOFTrackerMeasurements',
                ['CentralTrackingRecHits'],
                ['CentralWithoutTOFTrackerMeasurements'])
        app.add(meta.CollectionCollector_Measurement2D_true, 'CentralTrackerMeasurements',
                ['CentralWithoutTOFTrackerMeasurements', 'TOFBarrelClusterHits', 'TOFEndcapClusterHits'],
                ['CentralTrackerMeasurements'])
        app.add(tracking.CKFTracking, 'CentralCKFTruthSeededTrajectories',
                ['CentralTrackerTruthSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFTruthSeededActsTrackStatesUnfiltered', 'CentralCKFTruthSeededActsTracksUnfiltered'])
        app.add(tracking.ActsToTracks, 'CentralCKFTruthSeededTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededActsTrackStatesUnfiltered', 'CentralCKFTruthSeededActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTrajectoriesUnfiltered', 'CentralCKFTruthSeededTrackParametersUnfiltered', 'CentralCKFTruthSeededTracksUnfiltered', 'CentralCKFTruthSeededTrackUnfilteredLinks', 'CentralCKFTruthSeededTrackUnfilteredAssociations'])
        app.add(tracking.CKFTracking, 'CentralCKFTruthSeededTimeTrajectories',
                ['CentralTrackerTruthSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFTruthSeededTimeActsTrackStatesUnfiltered', 'CentralCKFTruthSeededTimeActsTracksUnfiltered'],
                timeReadouts=['MPGDBarrelHits', 'OuterMPGDBarrelHits', 'BackwardMPGDEndcapHits', 'ForwardMPGDEndcapHits', 'TOFBarrelHits', 'TOFEndcapHits'])
        app.add(tracking.ActsToTracks, 'CentralCKFTruthSeededTimeTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededTimeActsTrackStatesUnfiltered', 'CentralCKFTruthSeededTimeActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTimeTrajectoriesUnfiltered', 'CentralCKFTruthSeededTimeTrackParametersUnfiltered', 'CentralCKFTruthSeededTimeTracksUnfiltered', 'CentralCKFTruthSeededTimeTrackUnfilteredLinks', 'CentralCKFTruthSeededTimeTrackUnfilteredAssociations'])
        app.add(tracking.AmbiguitySolver, 'CentralCKFTruthSeededTimeAmbiguityResolutionSolver',
                ['CentralCKFTruthSeededTimeActsTrackStatesUnfiltered', 'CentralCKFTruthSeededTimeActsTracksUnfiltered'],
                ['CentralCKFTruthSeededTimeActsTrackStates', 'CentralCKFTruthSeededTimeActsTracks'])
        app.add(tracking.ActsToTracks, 'CentralCKFTruthSeededTimeTracks',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededTimeActsTrackStates', 'CentralCKFTruthSeededTimeActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTimeTrajectories', 'CentralCKFTruthSeededTimeTrackParameters', 'CentralCKFTruthSeededTimeTracks', 'CentralCKFTruthSeededTimeTrackLinks', 'CentralCKFTruthSeededTimeTrackAssociations'])
        app.add(tracking.AmbiguitySolver, 'TruthSeededAmbiguityResolutionSolver',
                ['CentralCKFTruthSeededActsTrackStatesUnfiltered', 'CentralCKFTruthSeededActsTracksUnfiltered'],
                ['CentralCKFTruthSeededActsTrackStates', 'CentralCKFTruthSeededActsTracks'])
        app.add(tracking.ActsToTracks, 'CentralCKFTruthSeededTracks',
                ['CentralTrackerMeasurements', 'CentralTrackerTruthSeeds', 'CentralCKFTruthSeededActsTrackStates', 'CentralCKFTruthSeededActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTruthSeededTrajectories', 'CentralCKFTruthSeededTrackParameters', 'CentralCKFTruthSeededTracks', 'CentralCKFTruthSeededTrackLinks', 'CentralCKFTruthSeededTrackAssociations'])
        app.add(tracking.TrackSeeding, 'CentralTrackSeeds',
                ['CentralTrackingRecHits'],
                ['CentralTrackSeeds', 'CentralTrackSeedParameters'])
        app.add(tracking.CKFTracking, 'CentralCKFTrajectories',
                ['CentralTrackSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFActsTrackStatesUnfiltered', 'CentralCKFActsTracksUnfiltered'])
        app.add(tracking.ActsToTracks, 'CentralCKFTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFActsTrackStatesUnfiltered', 'CentralCKFActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTrajectoriesUnfiltered', 'CentralCKFTrackParametersUnfiltered', 'CentralCKFTracksUnfiltered', 'CentralCKFTrackUnfilteredLinks', 'CentralCKFTrackUnfilteredAssociations'])
        app.add(tracking.CKFTracking, 'CentralCKFTimeTrajectories',
                ['CentralTrackSeeds', 'CentralTrackerMeasurements'],
                ['CentralCKFTimeActsTrackStatesUnfiltered', 'CentralCKFTimeActsTracksUnfiltered'],
                timeReadouts=['MPGDBarrelHits', 'OuterMPGDBarrelHits', 'BackwardMPGDEndcapHits', 'ForwardMPGDEndcapHits', 'TOFBarrelHits', 'TOFEndcapHits'])
        app.add(tracking.ActsToTracks, 'CentralCKFTimeTracksUnfiltered',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFTimeActsTrackStatesUnfiltered', 'CentralCKFTimeActsTracksUnfiltered', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTimeTrajectoriesUnfiltered', 'CentralCKFTimeTrackParametersUnfiltered', 'CentralCKFTimeTracksUnfiltered', 'CentralCKFTimeTrackUnfilteredLinks', 'CentralCKFTimeTrackUnfilteredAssociations'])
        app.add(tracking.AmbiguitySolver, 'CentralCKFTimeAmbiguityResolutionSolver',
                ['CentralCKFTimeActsTrackStatesUnfiltered', 'CentralCKFTimeActsTracksUnfiltered'],
                ['CentralCKFTimeActsTrackStates', 'CentralCKFTimeActsTracks'])
        app.add(tracking.ActsToTracks, 'CentralCKFTimeTracks',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFTimeActsTrackStates', 'CentralCKFTimeActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTimeTrajectories', 'CentralCKFTimeTrackParameters', 'CentralCKFTimeTracks', 'CentralCKFTimeTrackLinks', 'CentralCKFTimeTrackAssociations'])
        app.add(tracking.AmbiguitySolver, 'AmbiguityResolutionSolver',
                ['CentralCKFActsTrackStatesUnfiltered', 'CentralCKFActsTracksUnfiltered'],
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks'])
        app.add(tracking.ActsToTracks, 'CentralCKFTracks',
                ['CentralTrackerMeasurements', 'CentralTrackSeeds', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'CentralTrackingRawHitAssociations'],
                ['CentralCKFTrajectories', 'CentralCKFTrackParameters', 'CentralCKFTracks', 'CentralCKFTrackLinks', 'CentralCKFTrackAssociations'])
        app.add(tracking.TrackProjector, 'CentralTrackSegments',
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'CentralCKFTracks'],
                ['CentralTrackSegments'])
        app.add(tracking.IterativeVertexFinder, 'CentralTrackVertices',
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'ReconstructedChargedParticles'],
                ['CentralTrackVertices'])
        app.add(tracking.TrackPropagation, 'CalorimeterTrackPropagator',
                ['CentralCKFTracks', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks'],
                ['CalorimeterTrackProjections'],
                target_surfaces=[{'type': 'CylinderSurfaceConfig', 'id': 'BarrelDIRC_ID', 'rmin': 'DIRC_rmin', 'zmin': 'DIRC_offset - DIRC_length / 2.0', 'zmax': 'DIRC_offset + DIRC_length / 2.0'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapN_ID', 'zmin': '- EcalEndcapN_zmin', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapN_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapN_ID', 'zmin': '- EcalEndcapN_zmin - 50*mm', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapN_rmax'}, {'type': 'CylinderSurfaceConfig', 'id': 'EcalBarrel_ID', 'rmin': 'EcalBarrel_rmin', 'zmin': '- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)', 'zmax': '1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)'}, {'type': 'CylinderSurfaceConfig', 'id': 'EcalBarrel_ID', 'rmin': 'EcalBarrel_rmin + 50*mm', 'zmin': '- 1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)', 'zmax': '1.1*max(EcalBarrelBackward_zmax,EcalBarrelForward_zmax)'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapP_ID', 'zmin': 'EcalEndcapP_zmin', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapP_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'EcalEndcapP_ID', 'zmin': 'EcalEndcapP_zmin + 50*mm', 'rmin': 0.0, 'rmax': '1.1*EcalEndcapP_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'HcalEndcapN_ID', 'zmin': '- HcalEndcapN_zmin', 'rmin': 0.0, 'rmax': '1.1*HcalEndcapN_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'HcalEndcapN_ID', 'zmin': '- HcalEndcapN_zmin - 150*mm', 'rmin': 0.0, 'rmax': '1.1*HcalEndcapN_rmax'}, {'type': 'CylinderSurfaceConfig', 'id': 'HcalBarrel_ID', 'rmin': 'HcalBarrel_rmin', 'zmin': '- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)', 'zmax': '1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)'}, {'type': 'CylinderSurfaceConfig', 'id': 'HcalBarrel_ID', 'rmin': 'HcalBarrel_rmin + 150*mm', 'zmin': '- 1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)', 'zmax': '1.1*max(HcalBarrelBackward_zmax,HcalBarrelForward_zmax)'}, {'type': 'DiscSurfaceConfig', 'id': 'LFHCAL_ID', 'zmin': 'LFHCAL_zmin', 'rmin': 0.0, 'rmax': '1.1*LFHCAL_rmax'}, {'type': 'DiscSurfaceConfig', 'id': 'LFHCAL_ID', 'zmin': 'LFHCAL_zmin + 150*mm', 'rmin': 0.0, 'rmax': '1.1*LFHCAL_rmax'}])
        app.add(tracking.TrackerMeasurementFromHits, 'B0TrackerMeasurements',
                ['B0TrackerRecHits'],
                ['B0TrackerMeasurements'])
        app.add(tracking.CKFTracking, 'B0TrackerCKFTruthSeededTrajectories',
                ['B0TrackerTruthSeeds', 'B0TrackerMeasurements'],
                ['B0TrackerCKFTruthSeededActsTrackStatesUnfiltered', 'B0TrackerCKFTruthSeededActsTracksUnfiltered'],
                numMeasurementsMin=3)
        app.add(tracking.ActsToTracks, 'B0TrackerCKFTruthSeededTracksUnfiltered',
                ['B0TrackerMeasurements', 'B0TrackerTruthSeeds', 'B0TrackerCKFTruthSeededActsTrackStatesUnfiltered', 'B0TrackerCKFTruthSeededActsTracksUnfiltered', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTruthSeededTrajectoriesUnfiltered', 'B0TrackerCKFTruthSeededTrackParametersUnfiltered', 'B0TrackerCKFTruthSeededTracksUnfiltered', 'B0TrackerCKFTruthSeededTrackUnfilteredLinks', 'B0TrackerCKFTruthSeededTrackUnfilteredAssociations'])
        app.add(tracking.AmbiguitySolver, 'B0TrackerTruthSeededAmbiguityResolutionSolver',
                ['B0TrackerCKFTruthSeededActsTrackStatesUnfiltered', 'B0TrackerCKFTruthSeededActsTracksUnfiltered'],
                ['B0TrackerCKFTruthSeededActsTrackStates', 'B0TrackerCKFTruthSeededActsTracks'],
                n_measurements_min=3)
        app.add(tracking.ActsToTracks, 'B0TrackerCKFTruthSeededTracks',
                ['B0TrackerMeasurements', 'B0TrackerTruthSeeds', 'B0TrackerCKFTruthSeededActsTrackStates', 'B0TrackerCKFTruthSeededActsTracks', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTruthSeededTrajectories', 'B0TrackerCKFTruthSeededTrackParameters', 'B0TrackerCKFTruthSeededTracks', 'B0TrackerCKFTruthSeededTrackLinks', 'B0TrackerCKFTruthSeededTrackAssociations'])
        app.add(tracking.B0TrackSeeding, 'B0TrackerSeeds',
                ['B0TrackerRecHits'],
                ['B0TrackerSeeds', 'B0TrackerSeedParameters'])
        app.add(tracking.CKFTracking, 'B0TrackerCKFTrajectories',
                ['B0TrackerSeeds', 'B0TrackerMeasurements'],
                ['B0TrackerCKFActsTrackStatesUnfiltered', 'B0TrackerCKFActsTracksUnfiltered'],
                numMeasurementsMin=3)
        app.add(tracking.ActsToTracks, 'B0TrackerCKFTracksUnfiltered',
                ['B0TrackerMeasurements', 'B0TrackerSeeds', 'B0TrackerCKFActsTrackStatesUnfiltered', 'B0TrackerCKFActsTracksUnfiltered', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTrajectoriesUnfiltered', 'B0TrackerCKFTrackParametersUnfiltered', 'B0TrackerCKFTracksUnfiltered', 'B0TrackerCKFTrackUnfilteredLinks', 'B0TrackerCKFTrackUnfilteredAssociations'])
        app.add(tracking.AmbiguitySolver, 'B0TrackerAmbiguityResolutionSolver',
                ['B0TrackerCKFActsTrackStatesUnfiltered', 'B0TrackerCKFActsTracksUnfiltered'],
                ['B0TrackerCKFActsTrackStates', 'B0TrackerCKFActsTracks'],
                n_measurements_min=3)
        app.add(tracking.ActsToTracks, 'B0TrackerCKFTracks',
                ['B0TrackerMeasurements', 'B0TrackerSeeds', 'B0TrackerCKFActsTrackStates', 'B0TrackerCKFActsTracks', 'B0TrackerRawHitAssociations'],
                ['B0TrackerCKFTrajectories', 'B0TrackerCKFTrackParameters', 'B0TrackerCKFTracks', 'B0TrackerCKFTrackLinks', 'B0TrackerCKFTrackAssociations'])
        app.add(tracking.ActsTrackMerger, 'CentralAndB0TrackerCKFActsTracks',
                ['CentralCKFActsTrackStates', 'CentralCKFActsTracks', 'B0TrackerCKFActsTrackStates', 'B0TrackerCKFActsTracks'],
                ['CentralAndB0TrackerCKFActsTrackStates', 'CentralAndB0TrackerCKFActsTracks'])
        app.add(tracking.IterativeVertexFinder, 'CentralAndB0TrackVertices',
                ['CentralAndB0TrackerCKFActsTrackStates', 'CentralAndB0TrackerCKFActsTracks', 'ReconstructedChargedParticles'],
                ['CentralAndB0TrackVertices'])
        app.add(meta.CollectionCollector_Track_true, 'CombinedTracks',
                ['CentralCKFTracks', 'B0TrackerCKFTracks'],
                ['CombinedTracks'])
        app.add(tracking.SecondaryVertexFinder, 'PrimaryVerticesAMVF',
                ['ReconstructedParticles', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks'],
                ['PrimaryVerticesAMVF'])
        app.add(tracking.SecondaryVertexFinder, 'SecondaryVerticesAMVF',
                ['ReconstructedParticles', 'CentralCKFActsTrackStates', 'CentralCKFActsTracks'],
                ['SecondaryVerticesAMVF'],
                isPrimary=False,
                maxIterations=1000,
                tracksMaxZinterval=1000.0,
                useSeedConstraint=True,
                useTwoTrackSeeder=True)
        app.add(meta.CollectionCollector_MCRecoTrackParticleAssociation_true, 'CombinedTrackAssociations',
                ['CentralCKFTrackAssociations', 'B0TrackerCKFTrackAssociations'],
                ['CombinedTrackAssociations'])
        app.add(meta.CollectionCollector_Track_true, 'CombinedTruthSeededTracks',
                ['CentralCKFTruthSeededTracks', 'B0TrackerCKFTruthSeededTracks'],
                ['CombinedTruthSeededTracks'])
        app.add(meta.CollectionCollector_MCRecoTrackParticleAssociation_true, 'CombinedTruthSeededTrackAssociations',
                ['CentralCKFTruthSeededTrackAssociations', 'B0TrackerCKFTruthSeededTrackAssociations'],
                ['CombinedTruthSeededTrackAssociations'])
        app.add(meta.CollectionCollector_MCRecoTrackParticleLink_true, 'CombinedTruthSeededTrackLinks',
                ['CentralCKFTruthSeededTrackLinks', 'B0TrackerCKFTruthSeededTrackLinks'],
                ['CombinedTruthSeededTrackLinks'])
        app.add(tracking.TracksToParticles, 'ChargedTruthSeededParticlesWithAssociations',
                ['CombinedTruthSeededTracks', 'CombinedTruthSeededTrackAssociations'],
                ['ReconstructedTruthSeededChargedWithoutPIDParticles', 'ReconstructedTruthSeededChargedWithoutPIDParticleLinks', 'ReconstructedTruthSeededChargedWithoutPIDParticleAssociations'])
        app.add(tracking.TracksToParticles, 'ChargedParticlesWithAssociations',
                ['CombinedTracks', 'CombinedTrackAssociations'],
                ['ReconstructedChargedWithoutPIDParticles', 'ReconstructedChargedWithoutPIDParticleLinks', 'ReconstructedChargedWithoutPIDParticleAssociations'])
