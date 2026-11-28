# SPDX-License-Identifier: LGPL-3.0-or-later
"""particle_flow wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import calorimetry, meta, particle_flow, reco

PLUGIN = "particle_flow"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(meta.CollectionCollector_TrackClusterMatch_true, 'EcalTrackClusterMatches',
                ['EcalEndcapNTrackClusterMatches', 'EcalBarrelTrackClusterMatches', 'EcalEndcapPTrackClusterMatches'],
                ['EcalTrackClusterMatches'])
        app.add(meta.FilterMatching_Cluster_self_TrackClusterMatch_cluster, 'MatchedEcalClusters',
                ['EcalClusters', 'EcalTrackClusterMatches'],
                ['MatchedEcalClusters', 'UnmatchedEcalClusters'])
        app.add(meta.FilterMatching_MCRecoClusterParticleAssociation_rec_TrackClusterMatch_cluster, 'MatchedEcalClusterAssociations',
                ['EcalClusterAssociations', 'EcalTrackClusterMatches'],
                ['MatchedEcalClusterAssociations', 'UnmatchedEcalClusterAssociations'])
        app.add(reco.ClustersToParticles, 'ReconstructedNeutralParticlesZero',
                ['UnmatchedEcalClusters', 'UnmatchedEcalClusterAssociations'],
                ['ReconstructedNeutralParticlesZero', 'ReconstructedNeutralParticleZeroLinks', 'ReconstructedNeutralParticleZeroAssociations'])
        app.add(meta.CollectionCollector_ReconstructedParticle_false, 'ReconstructedParticlesZeroSubset',
                ['ReconstructedChargedParticles', 'ReconstructedNeutralParticlesZero'],
                ['ReconstructedParticlesZeroSubset'])
        app.add(meta.CollectionCollector_MCRecoParticleLink_false, 'ReconstructedParticleZeroLinksSubset',
                ['ReconstructedChargedParticleLinks', 'ReconstructedNeutralParticleZeroLinks'],
                ['ReconstructedParticleZeroLinksSubset'])
        app.add(meta.Cloner_ReconstructedParticle, 'ReconstructedParticlesZero',
                ['ReconstructedParticlesZeroSubset'],
                ['ReconstructedParticlesZero'])
        app.add(meta.CopyLinks_ReconstructedParticle_MCRecoParticleLink, 'ReconstructedParticleZeroLinks',
                ['ReconstructedParticlesZeroSubset', 'ReconstructedParticlesZero', 'ReconstructedParticleZeroLinksSubset'],
                ['ReconstructedParticleZeroLinks'])
        app.add(particle_flow.TrackClusterMergeSplitter, 'EcalEndcapNSplitMergeProtoClusters',
                ['EcalEndcapNTrackClusterMatches', 'EcalEndcapNClustersWithoutPID', 'CalorimeterTrackProjections'],
                ['EcalEndcapNSplitMergeProtoClusters', 'EcalEndcapNTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                sigEP=0.10,
                drAdd=0.1)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalEndcapNSplitMergeClustersWithoutShapes',
                ['EcalEndcapNSplitMergeProtoClusters', 'EcalEndcapNRawHitLinks', 'EcalEndcapNRawHitAssociations'],
                ['EcalEndcapNSplitMergeClustersWithoutShapes', 'EcalEndcapNSplitMergeClusterLinksWithoutShapes', 'EcalEndcapNSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'EcalEndcapNSplitMergeClusters',
                ['EcalEndcapNSplitMergeClustersWithoutShapes', 'EcalEndcapNSplitMergeClusterLinksWithoutShapes'],
                ['EcalEndcapNSplitMergeClusters', 'EcalEndcapNSplitMergeClusterLinks', 'EcalEndcapNSplitMergeClusterAssociations'],
                energyWeight="log")
        app.add(particle_flow.TrackClusterMergeSplitter, 'HcalEndcapNSplitMergeProtoClusters',
                ['HcalEndcapNTrackClusterMatches', 'HcalEndcapNClusters', 'CalorimeterTrackProjections'],
                ['HcalEndcapNSplitMergeProtoClusters', 'HcalEndcapNTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                avgEP=0.60,
                sigEP=0.40)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalEndcapNSplitMergeClustersWithoutShapes',
                ['HcalEndcapNSplitMergeProtoClusters', 'HcalEndcapNRawHitLinks', 'HcalEndcapNRawHitAssociations'],
                ['HcalEndcapNSplitMergeClustersWithoutShapes', 'HcalEndcapNSplitMergeClusterLinksWithoutShapes', 'HcalEndcapNSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterClusterShape, 'HcalEndcapNSplitMergeClusters',
                ['HcalEndcapNSplitMergeClustersWithoutShapes', 'HcalEndcapNSplitMergeClusterLinksWithoutShapes'],
                ['HcalEndcapNSplitMergeClusters', 'HcalEndcapNSplitMergeClusterLinks', 'HcalEndcapNSplitMergeClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(particle_flow.TrackClusterMergeSplitter, 'HcalBarrelSplitMergeProtoClusters',
                ['HcalBarrelTrackClusterMatches', 'HcalBarrelClustersWithoutShapes', 'CalorimeterTrackProjections'],
                ['HcalBarrelSplitMergeProtoClusters', 'HcalBarrelTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                avgEP=0.50,
                sigEP=0.25)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'HcalBarrelSplitMergeClustersWithoutShapes',
                ['HcalBarrelSplitMergeProtoClusters', 'HcalBarrelRawHitLinks', 'HcalBarrelRawHitAssociations'],
                ['HcalBarrelSplitMergeClustersWithoutShapes', 'HcalBarrelSplitMergeClusterLinksWithoutShapes', 'HcalBarrelSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(calorimetry.CalorimeterClusterShape, 'HcalBarrelSplitMergeClusters',
                ['HcalBarrelSplitMergeClustersWithoutShapes', 'HcalBarrelSplitMergeClusterLinksWithoutShapes'],
                ['HcalBarrelSplitMergeClusters', 'HcalBarrelSplitMergeClusterLinks', 'HcalBarrelSplitMergeClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(particle_flow.TrackClusterMergeSplitter, 'EcalEndcapPSplitMergeProtoClusters',
                ['EcalEndcapPTrackClusterMatches', 'EcalEndcapPClusters', 'CalorimeterTrackProjections'],
                ['EcalEndcapPSplitMergeProtoClusters', 'EcalEndcapPTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                sigEP=0.10,
                drAdd=0.30)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'EcalEndcapPSplitMergeClustersWithoutShapes',
                ['EcalEndcapPSplitMergeProtoClusters', 'EcalEndcapPRawHitLinks', 'EcalEndcapPRawHitAssociations'],
                ['EcalEndcapPSplitMergeClustersWithoutShapes', 'EcalEndcapPSplitMergeClusterLinksWithoutShapes', 'EcalEndcapPSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(calorimetry.CalorimeterClusterShape, 'EcalEndcapPSplitMergeClusters',
                ['EcalEndcapPSplitMergeClustersWithoutShapes', 'EcalEndcapPSplitMergeClusterLinksWithoutShapes'],
                ['EcalEndcapPSplitMergeClusters', 'EcalEndcapPSplitMergeClusterLinks', 'EcalEndcapPSplitMergeClusterAssociations'],
                energyWeight="log")
        app.add(particle_flow.TrackClusterMergeSplitter, 'LFHCALSplitMergeProtoClusters',
                ['LFHCALTrackClusterMatches', 'LFHCALClusters', 'CalorimeterTrackProjections'],
                ['LFHCALSplitMergeProtoClusters', 'LFHCALTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                avgEP=0.50,
                sigEP=0.25,
                drAdd=0.30)
        app.add(calorimetry.CalorimeterClusterRecoCoG, 'LFHCALSplitMergeClustersWithoutShapes',
                ['LFHCALSplitMergeProtoClusters', 'LFHCALRawHitLinks', 'LFHCALRawHitAssociations'],
                ['LFHCALSplitMergeClustersWithoutShapes', 'LFHCALSplitMergeClusterLinksWithoutShapes', 'LFHCALSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=4.5)
        app.add(calorimetry.CalorimeterClusterShape, 'LFHCALSplitMergeClusters',
                ['LFHCALSplitMergeClustersWithoutShapes', 'LFHCALSplitMergeClusterLinksWithoutShapes'],
                ['LFHCALSplitMergeClusters', 'LFHCALSplitMergeClusterLinks', 'LFHCALSplitMergeClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=4.5)
        app.add(particle_flow.TrackProtoClusterMatchPromoter, 'EcalEndcapNTrackSplitMergeClusterMatches',
                ['EcalEndcapNTrackSplitMergeProtoClusterLinks', 'EcalEndcapNSplitMergeProtoClusters', 'EcalEndcapNSplitMergeClusters'],
                ['EcalEndcapNTrackSplitMergeClusterMatches'])
        app.add(particle_flow.TrackProtoClusterMatchPromoter, 'HcalEndcapNTrackSplitMergeClusterMatches',
                ['HcalEndcapNTrackSplitMergeProtoClusterLinks', 'HcalEndcapNSplitMergeProtoClusters', 'HcalEndcapNSplitMergeClusters'],
                ['HcalEndcapNTrackSplitMergeClusterMatches'])
        app.add(particle_flow.TrackProtoClusterMatchPromoter, 'HcalBarrelTrackSplitMergeClusterMatches',
                ['HcalBarrelTrackSplitMergeProtoClusterLinks', 'HcalBarrelSplitMergeProtoClusters', 'HcalBarrelSplitMergeClusters'],
                ['HcalBarrelTrackSplitMergeClusterMatches'])
        app.add(particle_flow.TrackProtoClusterMatchPromoter, 'EcalEndcapPTrackSplitMergeClusterMatches',
                ['EcalEndcapPTrackSplitMergeProtoClusterLinks', 'EcalEndcapPSplitMergeProtoClusters', 'EcalEndcapPSplitMergeClusters'],
                ['EcalEndcapPTrackSplitMergeClusterMatches'])
        app.add(particle_flow.TrackProtoClusterMatchPromoter, 'LFHCALTrackSplitMergeClusterMatches',
                ['LFHCALTrackSplitMergeProtoClusterLinks', 'LFHCALSplitMergeProtoClusters', 'LFHCALSplitMergeClusters'],
                ['LFHCALTrackSplitMergeClusterMatches'])
        app.add(meta.SubDivideCollection_TrackSegment, 'EndcapPBarrelEndcapNCalorimeterTrackProjections',
                ['CalorimeterTrackProjections'],
                ['EcalEndcapNCalorimeterTrackProjections', 'HcalEndcapNCalorimeterTrackProjections', 'EcalBarrelCalorimeterTrackProjections', 'HcalBarrelCalorimeterTrackProjections', 'EcalEndcapPCalorimeterTrackProjections', 'LFHCALTrackProjections', 'HcalEndcapPInsertCalorimeterTrackProjections'],
                function=meta.TrackPointSystemSplit([103, 113, 101, 111, 102, 116, 115]))
        app.add(particle_flow.TrackClusterSubtractor, 'EcalEndcapNRemnantClusters',
                ['EcalEndcapNTrackClusterMatches', 'EcalEndcapNClusters', 'EcalEndcapNCalorimeterTrackProjections'],
                ['EcalEndcapNRemnantClusters', 'EcalEndcapNExpectedClusters', 'EcalEndcapNTrackExpectedClusterLinks', 'EcalEndcapNTrackExpectedClusterMatches'])
        app.add(particle_flow.TrackClusterSubtractor, 'HcalEndcapNRemnantClusters',
                ['HcalEndcapNTrackClusterMatches', 'HcalEndcapNClusters', 'HcalEndcapNCalorimeterTrackProjections'],
                ['HcalEndcapNRemnantClusters', 'HcalEndcapNExpectedClusters', 'HcalEndcapNTrackExpectedClusterLinks', 'HcalEndcapNTrackExpectedClusterMatches'])
        app.add(particle_flow.TrackClusterSubtractor, 'EcalBarrelRemnantClusters',
                ['EcalBarrelTrackClusterMatches', 'EcalBarrelClusters', 'EcalBarrelCalorimeterTrackProjections'],
                ['EcalBarrelRemnantClusters', 'EcalBarrelExpectedClusters', 'EcalBarrelTrackExpectedClusterLinks', 'EcalBarrelTrackExpectedClusterMatches'])
        app.add(particle_flow.TrackClusterSubtractor, 'HcalBarrelRemnantClusters',
                ['HcalBarrelTrackClusterMatches', 'HcalBarrelClusters', 'HcalBarrelCalorimeterTrackProjections'],
                ['HcalBarrelRemnantClusters', 'HcalBarrelExpectedClusters', 'HcalBarrelTrackExpectedClusterLinks', 'HcalBarrelTrackExpectedClusterMatches'])
        app.add(particle_flow.TrackClusterSubtractor, 'EcalEndcapPRemnantClusters',
                ['EcalEndcapPTrackClusterMatches', 'EcalEndcapPClusters', 'EcalEndcapPCalorimeterTrackProjections'],
                ['EcalEndcapPRemnantClusters', 'EcalEndcapPExpectedClusters', 'EcalEndcapPTrackExpectedClusterLinks', 'EcalEndcapPTrackExpectedClusterMatches'])
        app.add(particle_flow.TrackClusterSubtractor, 'LFHCALRemnantClusters',
                ['LFHCALTrackSplitMergeClusterMatches', 'LFHCALClusters', 'LFHCALTrackProjections'],
                ['LFHCALRemnantClusters', 'LFHCALExpectedClusters', 'LFHCALTrackExpectedClusterLinks', 'LFHCALTrackExpectedClusterMatches'])
        app.add(particle_flow.TrackClusterSubtractor, 'HcalEndcapPInsertRemnantClusters',
                ['HcalEndcapPInsertTrackClusterMatches', 'HcalEndcapPInsertClusters', 'HcalEndcapPInsertCalorimeterTrackProjections'],
                ['HcalEndcapPInsertRemnantClusters', 'HcalEndcapPInsertExpectedClusters', 'HcalEndcapPInsertTrackExpectedClusterLinks', 'HcalEndcapPInsertTrackExpectedClusterMatches'])
        app.add(meta.CollectionCollector_TrackClusterMatch_false, 'EndcapNTrackClusterMatches',
                ['EcalEndcapNTrackClusterMatches', 'HcalEndcapNTrackClusterMatches'],
                ['EndcapNTrackClusterMatches'])
        app.add(particle_flow.ChargedCandidateMaker, 'EndcapNChargedCandidateParticlesAlpha',
                ['EndcapNTrackClusterMatches'],
                ['EndcapNChargedCandidateParticlesAlpha'])
        app.add(meta.CollectionCollector_TrackClusterMatch_false, 'BarrelTrackClusterMatches',
                ['EcalBarrelTrackClusterMatches', 'HcalBarrelTrackClusterMatches'],
                ['BarrelTrackClusterMatches'])
        app.add(particle_flow.ChargedCandidateMaker, 'BarrelChargedCandidateParticlesAlpha',
                ['BarrelTrackClusterMatches'],
                ['BarrelChargedCandidateParticlesAlpha'])
        app.add(meta.CollectionCollector_TrackClusterMatch_false, 'EndcapPTrackClusterMatches',
                ['EcalEndcapPTrackClusterMatches', 'LFHCALTrackClusterMatches', 'HcalEndcapPInsertTrackClusterMatches'],
                ['EndcapPTrackClusterMatches'])
        app.add(particle_flow.ChargedCandidateMaker, 'EndcapPChargedCandidateParticlesAlpha',
                ['EndcapPTrackClusterMatches'],
                ['EndcapPChargedCandidateParticlesAlpha'])
        app.add(particle_flow.CaloRemnantCombiner, 'EndcapNNeutralCandidateParticlesAlpha',
                ['EcalEndcapNRemnantClusters', 'HcalEndcapNRemnantClusters'],
                ['EndcapNNeutralCandidateParticlesAlpha'])
        app.add(particle_flow.CaloRemnantCombiner, 'BarrelNeutralCandidateParticlesAlpha',
                ['EcalBarrelRemnantClusters', 'HcalBarrelRemnantClusters'],
                ['BarrelNeutralCandidateParticlesAlpha'])
        app.add(meta.CollectionCollector_Cluster_false, 'CombinedHcalEndcapPRemnantClusters',
                ['LFHCALRemnantClusters', 'HcalEndcapPInsertRemnantClusters'],
                ['CombinedHcalEndcapPRemnantClusters'])
        app.add(particle_flow.CaloRemnantCombiner, 'EndcapPNeutralCandidateParticlesAlpha',
                ['EcalEndcapPRemnantClusters', 'CombinedHcalEndcapPRemnantClusters'],
                ['EndcapPNeutralCandidateParticlesAlpha'])
