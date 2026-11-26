# SPDX-License-Identifier: LGPL-3.0-or-later
"""particle_flow wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "particle_flow"


def configure(app, plugin=PLUGIN):
    T = bindings("particle_flow")
    with app.plugin(plugin):
        app.add(T.CollectionCollector_TrackClusterMatch_true, 'EcalTrackClusterMatches',
                ['EcalEndcapNTrackClusterMatches', 'EcalBarrelTrackClusterMatches', 'EcalEndcapPTrackClusterMatches'],
                ['EcalTrackClusterMatches'])
        app.add(T.FilterMatching_Cluster_self_TrackClusterMatch_cluster, 'MatchedEcalClusters',
                ['EcalClusters', 'EcalTrackClusterMatches'],
                ['MatchedEcalClusters', 'UnmatchedEcalClusters'])
        app.add(T.FilterMatching_MCRecoClusterParticleAssociation_rec_TrackClusterMatch_cluster, 'MatchedEcalClusterAssociations',
                ['EcalClusterAssociations', 'EcalTrackClusterMatches'],
                ['MatchedEcalClusterAssociations', 'UnmatchedEcalClusterAssociations'])
        app.add(T.ClustersToParticles, 'ReconstructedNeutralParticlesZero',
                ['UnmatchedEcalClusters', 'UnmatchedEcalClusterAssociations'],
                ['ReconstructedNeutralParticlesZero', 'ReconstructedNeutralParticleZeroLinks', 'ReconstructedNeutralParticleZeroAssociations'])
        app.add(T.CollectionCollector_ReconstructedParticle_false, 'ReconstructedParticlesZeroSubset',
                ['ReconstructedChargedParticles', 'ReconstructedNeutralParticlesZero'],
                ['ReconstructedParticlesZeroSubset'])
        app.add(T.CollectionCollector_MCRecoParticleLink_false, 'ReconstructedParticleZeroLinksSubset',
                ['ReconstructedChargedParticleLinks', 'ReconstructedNeutralParticleZeroLinks'],
                ['ReconstructedParticleZeroLinksSubset'])
        app.add(T.Cloner_ReconstructedParticle, 'ReconstructedParticlesZero',
                ['ReconstructedParticlesZeroSubset'],
                ['ReconstructedParticlesZero'])
        app.add(T.CopyLinks_ReconstructedParticle_MCRecoParticleLink, 'ReconstructedParticleZeroLinks',
                ['ReconstructedParticlesZeroSubset', 'ReconstructedParticlesZero', 'ReconstructedParticleZeroLinksSubset'],
                ['ReconstructedParticleZeroLinks'])
        app.add(T.TrackClusterMergeSplitter, 'EcalEndcapNSplitMergeProtoClusters',
                ['EcalEndcapNTrackClusterMatches', 'EcalEndcapNClustersWithoutPID', 'CalorimeterTrackProjections'],
                ['EcalEndcapNSplitMergeProtoClusters', 'EcalEndcapNTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                sigEP=0.10,
                drAdd=0.1)
        app.add(T.CalorimeterClusterRecoCoG, 'EcalEndcapNSplitMergeClustersWithoutShapes',
                ['EcalEndcapNSplitMergeProtoClusters', 'EcalEndcapNRawHitLinks', 'EcalEndcapNRawHitAssociations'],
                ['EcalEndcapNSplitMergeClustersWithoutShapes', 'EcalEndcapNSplitMergeClusterLinksWithoutShapes', 'EcalEndcapNSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(T.CalorimeterClusterShape, 'EcalEndcapNSplitMergeClusters',
                ['EcalEndcapNSplitMergeClustersWithoutShapes', 'EcalEndcapNSplitMergeClusterLinksWithoutShapes'],
                ['EcalEndcapNSplitMergeClusters', 'EcalEndcapNSplitMergeClusterLinks', 'EcalEndcapNSplitMergeClusterAssociations'],
                energyWeight="log")
        app.add(T.TrackClusterMergeSplitter, 'HcalEndcapNSplitMergeProtoClusters',
                ['HcalEndcapNTrackClusterMatches', 'HcalEndcapNClusters', 'CalorimeterTrackProjections'],
                ['HcalEndcapNSplitMergeProtoClusters', 'HcalEndcapNTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                avgEP=0.60,
                sigEP=0.40)
        app.add(T.CalorimeterClusterRecoCoG, 'HcalEndcapNSplitMergeClustersWithoutShapes',
                ['HcalEndcapNSplitMergeProtoClusters', 'HcalEndcapNRawHitLinks', 'HcalEndcapNRawHitAssociations'],
                ['HcalEndcapNSplitMergeClustersWithoutShapes', 'HcalEndcapNSplitMergeClusterLinksWithoutShapes', 'HcalEndcapNSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'HcalEndcapNSplitMergeClusters',
                ['HcalEndcapNSplitMergeClustersWithoutShapes', 'HcalEndcapNSplitMergeClusterLinksWithoutShapes'],
                ['HcalEndcapNSplitMergeClusters', 'HcalEndcapNSplitMergeClusterLinks', 'HcalEndcapNSplitMergeClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.TrackClusterMergeSplitter, 'HcalBarrelSplitMergeProtoClusters',
                ['HcalBarrelTrackClusterMatches', 'HcalBarrelClustersWithoutShapes', 'CalorimeterTrackProjections'],
                ['HcalBarrelSplitMergeProtoClusters', 'HcalBarrelTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                avgEP=0.50,
                sigEP=0.25)
        app.add(T.CalorimeterClusterRecoCoG, 'HcalBarrelSplitMergeClustersWithoutShapes',
                ['HcalBarrelSplitMergeProtoClusters', 'HcalBarrelRawHitLinks', 'HcalBarrelRawHitAssociations'],
                ['HcalBarrelSplitMergeClustersWithoutShapes', 'HcalBarrelSplitMergeClusterLinksWithoutShapes', 'HcalBarrelSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.CalorimeterClusterShape, 'HcalBarrelSplitMergeClusters',
                ['HcalBarrelSplitMergeClustersWithoutShapes', 'HcalBarrelSplitMergeClusterLinksWithoutShapes'],
                ['HcalBarrelSplitMergeClusters', 'HcalBarrelSplitMergeClusterLinks', 'HcalBarrelSplitMergeClusterAssociations'],
                energyWeight="log",
                logWeightBase=6.2)
        app.add(T.TrackClusterMergeSplitter, 'EcalEndcapPSplitMergeProtoClusters',
                ['EcalEndcapPTrackClusterMatches', 'EcalEndcapPClusters', 'CalorimeterTrackProjections'],
                ['EcalEndcapPSplitMergeProtoClusters', 'EcalEndcapPTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                sigEP=0.10,
                drAdd=0.30)
        app.add(T.CalorimeterClusterRecoCoG, 'EcalEndcapPSplitMergeClustersWithoutShapes',
                ['EcalEndcapPSplitMergeProtoClusters', 'EcalEndcapPRawHitLinks', 'EcalEndcapPRawHitAssociations'],
                ['EcalEndcapPSplitMergeClustersWithoutShapes', 'EcalEndcapPSplitMergeClusterLinksWithoutShapes', 'EcalEndcapPSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log")
        app.add(T.CalorimeterClusterShape, 'EcalEndcapPSplitMergeClusters',
                ['EcalEndcapPSplitMergeClustersWithoutShapes', 'EcalEndcapPSplitMergeClusterLinksWithoutShapes'],
                ['EcalEndcapPSplitMergeClusters', 'EcalEndcapPSplitMergeClusterLinks', 'EcalEndcapPSplitMergeClusterAssociations'],
                energyWeight="log")
        app.add(T.TrackClusterMergeSplitter, 'LFHCALSplitMergeProtoClusters',
                ['LFHCALTrackClusterMatches', 'LFHCALClusters', 'CalorimeterTrackProjections'],
                ['LFHCALSplitMergeProtoClusters', 'LFHCALTrackSplitMergeProtoClusterLinks'],
                minSigCut=-2.0,
                avgEP=0.50,
                sigEP=0.25,
                drAdd=0.30)
        app.add(T.CalorimeterClusterRecoCoG, 'LFHCALSplitMergeClustersWithoutShapes',
                ['LFHCALSplitMergeProtoClusters', 'LFHCALRawHitLinks', 'LFHCALRawHitAssociations'],
                ['LFHCALSplitMergeClustersWithoutShapes', 'LFHCALSplitMergeClusterLinksWithoutShapes', 'LFHCALSplitMergeClusterAssociationsWithoutShapes'],
                energyWeight="log",
                logWeightBase=4.5)
        app.add(T.CalorimeterClusterShape, 'LFHCALSplitMergeClusters',
                ['LFHCALSplitMergeClustersWithoutShapes', 'LFHCALSplitMergeClusterLinksWithoutShapes'],
                ['LFHCALSplitMergeClusters', 'LFHCALSplitMergeClusterLinks', 'LFHCALSplitMergeClusterAssociations'],
                longitudinalShowerInfoAvailable=True,
                energyWeight="log",
                logWeightBase=4.5)
        app.add(T.TrackProtoClusterMatchPromoter, 'EcalEndcapNTrackSplitMergeClusterMatches',
                ['EcalEndcapNTrackSplitMergeProtoClusterLinks', 'EcalEndcapNSplitMergeProtoClusters', 'EcalEndcapNSplitMergeClusters'],
                ['EcalEndcapNTrackSplitMergeClusterMatches'])
        app.add(T.TrackProtoClusterMatchPromoter, 'HcalEndcapNTrackSplitMergeClusterMatches',
                ['HcalEndcapNTrackSplitMergeProtoClusterLinks', 'HcalEndcapNSplitMergeProtoClusters', 'HcalEndcapNSplitMergeClusters'],
                ['HcalEndcapNTrackSplitMergeClusterMatches'])
        app.add(T.TrackProtoClusterMatchPromoter, 'HcalBarrelTrackSplitMergeClusterMatches',
                ['HcalBarrelTrackSplitMergeProtoClusterLinks', 'HcalBarrelSplitMergeProtoClusters', 'HcalBarrelSplitMergeClusters'],
                ['HcalBarrelTrackSplitMergeClusterMatches'])
        app.add(T.TrackProtoClusterMatchPromoter, 'EcalEndcapPTrackSplitMergeClusterMatches',
                ['EcalEndcapPTrackSplitMergeProtoClusterLinks', 'EcalEndcapPSplitMergeProtoClusters', 'EcalEndcapPSplitMergeClusters'],
                ['EcalEndcapPTrackSplitMergeClusterMatches'])
        app.add(T.TrackProtoClusterMatchPromoter, 'LFHCALTrackSplitMergeClusterMatches',
                ['LFHCALTrackSplitMergeProtoClusterLinks', 'LFHCALSplitMergeProtoClusters', 'LFHCALSplitMergeClusters'],
                ['LFHCALTrackSplitMergeClusterMatches'])
        app.add(T.SubDivideCollection_TrackSegment, 'EndcapPBarrelEndcapNCalorimeterTrackProjections',
                ['CalorimeterTrackProjections'],
                ['EcalEndcapNCalorimeterTrackProjections', 'HcalEndcapNCalorimeterTrackProjections', 'EcalBarrelCalorimeterTrackProjections', 'HcalBarrelCalorimeterTrackProjections', 'EcalEndcapPCalorimeterTrackProjections', 'LFHCALTrackProjections', 'HcalEndcapPInsertCalorimeterTrackProjections'],
                function=T.TrackPointSystemSplit([103, 113, 101, 111, 102, 116, 115]))
        app.add(T.TrackClusterSubtractor, 'EcalEndcapNRemnantClusters',
                ['EcalEndcapNTrackClusterMatches', 'EcalEndcapNClusters', 'EcalEndcapNCalorimeterTrackProjections'],
                ['EcalEndcapNRemnantClusters', 'EcalEndcapNExpectedClusters', 'EcalEndcapNTrackExpectedClusterLinks', 'EcalEndcapNTrackExpectedClusterMatches'])
        app.add(T.TrackClusterSubtractor, 'HcalEndcapNRemnantClusters',
                ['HcalEndcapNTrackClusterMatches', 'HcalEndcapNClusters', 'HcalEndcapNCalorimeterTrackProjections'],
                ['HcalEndcapNRemnantClusters', 'HcalEndcapNExpectedClusters', 'HcalEndcapNTrackExpectedClusterLinks', 'HcalEndcapNTrackExpectedClusterMatches'])
        app.add(T.TrackClusterSubtractor, 'EcalBarrelRemnantClusters',
                ['EcalBarrelTrackClusterMatches', 'EcalBarrelClusters', 'EcalBarrelCalorimeterTrackProjections'],
                ['EcalBarrelRemnantClusters', 'EcalBarrelExpectedClusters', 'EcalBarrelTrackExpectedClusterLinks', 'EcalBarrelTrackExpectedClusterMatches'])
        app.add(T.TrackClusterSubtractor, 'HcalBarrelRemnantClusters',
                ['HcalBarrelTrackClusterMatches', 'HcalBarrelClusters', 'HcalBarrelCalorimeterTrackProjections'],
                ['HcalBarrelRemnantClusters', 'HcalBarrelExpectedClusters', 'HcalBarrelTrackExpectedClusterLinks', 'HcalBarrelTrackExpectedClusterMatches'])
        app.add(T.TrackClusterSubtractor, 'EcalEndcapPRemnantClusters',
                ['EcalEndcapPTrackClusterMatches', 'EcalEndcapPClusters', 'EcalEndcapPCalorimeterTrackProjections'],
                ['EcalEndcapPRemnantClusters', 'EcalEndcapPExpectedClusters', 'EcalEndcapPTrackExpectedClusterLinks', 'EcalEndcapPTrackExpectedClusterMatches'])
        app.add(T.TrackClusterSubtractor, 'LFHCALRemnantClusters',
                ['LFHCALTrackSplitMergeClusterMatches', 'LFHCALClusters', 'LFHCALTrackProjections'],
                ['LFHCALRemnantClusters', 'LFHCALExpectedClusters', 'LFHCALTrackExpectedClusterLinks', 'LFHCALTrackExpectedClusterMatches'])
        app.add(T.TrackClusterSubtractor, 'HcalEndcapPInsertRemnantClusters',
                ['HcalEndcapPInsertTrackClusterMatches', 'HcalEndcapPInsertClusters', 'HcalEndcapPInsertCalorimeterTrackProjections'],
                ['HcalEndcapPInsertRemnantClusters', 'HcalEndcapPInsertExpectedClusters', 'HcalEndcapPInsertTrackExpectedClusterLinks', 'HcalEndcapPInsertTrackExpectedClusterMatches'])
        app.add(T.CollectionCollector_TrackClusterMatch_false, 'EndcapNTrackClusterMatches',
                ['EcalEndcapNTrackClusterMatches', 'HcalEndcapNTrackClusterMatches'],
                ['EndcapNTrackClusterMatches'])
        app.add(T.ChargedCandidateMaker, 'EndcapNChargedCandidateParticlesAlpha',
                ['EndcapNTrackClusterMatches'],
                ['EndcapNChargedCandidateParticlesAlpha'])
        app.add(T.CollectionCollector_TrackClusterMatch_false, 'BarrelTrackClusterMatches',
                ['EcalBarrelTrackClusterMatches', 'HcalBarrelTrackClusterMatches'],
                ['BarrelTrackClusterMatches'])
        app.add(T.ChargedCandidateMaker, 'BarrelChargedCandidateParticlesAlpha',
                ['BarrelTrackClusterMatches'],
                ['BarrelChargedCandidateParticlesAlpha'])
        app.add(T.CollectionCollector_TrackClusterMatch_false, 'EndcapPTrackClusterMatches',
                ['EcalEndcapPTrackClusterMatches', 'LFHCALTrackClusterMatches', 'HcalEndcapPInsertTrackClusterMatches'],
                ['EndcapPTrackClusterMatches'])
        app.add(T.ChargedCandidateMaker, 'EndcapPChargedCandidateParticlesAlpha',
                ['EndcapPTrackClusterMatches'],
                ['EndcapPChargedCandidateParticlesAlpha'])
        app.add(T.CaloRemnantCombiner, 'EndcapNNeutralCandidateParticlesAlpha',
                ['EcalEndcapNRemnantClusters', 'HcalEndcapNRemnantClusters'],
                ['EndcapNNeutralCandidateParticlesAlpha'])
        app.add(T.CaloRemnantCombiner, 'BarrelNeutralCandidateParticlesAlpha',
                ['EcalBarrelRemnantClusters', 'HcalBarrelRemnantClusters'],
                ['BarrelNeutralCandidateParticlesAlpha'])
        app.add(T.CollectionCollector_Cluster_false, 'CombinedHcalEndcapPRemnantClusters',
                ['LFHCALRemnantClusters', 'HcalEndcapPInsertRemnantClusters'],
                ['CombinedHcalEndcapPRemnantClusters'])
        app.add(T.CaloRemnantCombiner, 'EndcapPNeutralCandidateParticlesAlpha',
                ['EcalEndcapPRemnantClusters', 'CombinedHcalEndcapPRemnantClusters'],
                ['EndcapPNeutralCandidateParticlesAlpha'])
