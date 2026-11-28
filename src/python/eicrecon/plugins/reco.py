# SPDX-License-Identifier: LGPL-3.0-or-later
"""reco wiring, converted from the former C++ InitPlugin.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from ..factories import meta, reco

PLUGIN = "reco"


def configure(app, plugin=PLUGIN):
    with app.plugin(plugin):
        app.add(meta.FilterMatching_MCRecoParticleLink_to_MCParticle_self, 'MCScatteredElectronLinks',
                ['ReconstructedChargedParticleLinks', 'MCScatteredElectrons'],
                ['MCScatteredElectronLinks', 'MCNonScatteredElectronLinks'])
        app.add(meta.FilterMatching_MCRecoParticleAssociation_sim_MCParticle_self, 'MCScatteredElectronAssociations',
                ['ReconstructedChargedParticleAssociations', 'MCScatteredElectrons'],
                ['MCScatteredElectronAssociations', 'MCNonScatteredElectronAssociations'])
        app.add(reco.MC2ReconstructedParticle, 'GeneratedParticles',
                ['MCParticles'],
                ['GeneratedParticles'])
        app.add(meta.CollectionCollector_Cluster_true, 'EcalClusters',
                ['EcalEndcapNClusters', 'EcalBarrelClusters', 'EcalEndcapPClusters'],
                ['EcalClusters'])
        app.add(meta.CollectionCollector_MCRecoClusterParticleAssociation_true, 'EcalClusterAssociations',
                ['EcalEndcapNClusterAssociations', 'EcalBarrelClusterAssociations', 'EcalEndcapPClusterAssociations'],
                ['EcalClusterAssociations'])
        app.add(reco.MatchClusters, 'ReconstructedParticlesWithAssoc',
                ['MCParticles', 'ReconstructedChargedParticles', 'ReconstructedChargedParticleAssociations', 'EcalClusters', 'EcalClusterAssociations'],
                ['ReconstructedParticles', 'ReconstructedParticleLinks', 'ReconstructedParticleAssociations'])
        app.add(reco.InclusiveKinematicsTruthBare, 'InclusiveKinematicsTruthBare',
                ['MCParticles'],
                ['InclusiveKinematicsTruthBare'])
        app.add(meta.CollectionCollector_InclusiveKinematics, 'InclusiveKinematicsTruth',
                ['InclusiveKinematicsTruthBare'],
                ['InclusiveKinematicsTruth'])
        app.add(reco.InclusiveKinematicsTruthBorn, 'InclusiveKinematicsTruthBorn',
                ['MCParticles'],
                ['InclusiveKinematicsTruthBorn'])
        app.add(reco.InclusiveKinematicsTruthDressed, 'InclusiveKinematicsTruthDressed',
                ['MCParticles'],
                ['InclusiveKinematicsTruthDressed', 'ScatteredElectronTruthDressed', 'ScatteredElectronTruthDressedConstituents'])
        app.add(reco.InclusiveKinematicsReconstructed_InclusiveKinematicsElectron, 'InclusiveKinematicsElectron',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsElectron'])
        app.add(reco.ScatteredElectronsTruth, 'ScatteredElectronsTruth',
                ['MCParticles', 'ReconstructedChargedParticles', 'ReconstructedChargedParticleAssociations'],
                ['ScatteredElectronsTruth'])
        app.add(reco.HadronicFinalState_HadronicFinalState, 'HadronicFinalState',
                ['MCBeamElectrons', 'MCBeamHadrons', 'MCParticles', 'ReconstructedParticles', 'ReconstructedParticleAssociations'],
                ['HadronicFinalState'])
        app.add(reco.InclusiveKinematicsReconstructed_InclusiveKinematicsJB, 'InclusiveKinematicsJB',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsJB'])
        app.add(reco.InclusiveKinematicsReconstructed_InclusiveKinematicsDA, 'InclusiveKinematicsDA',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsDA'])
        app.add(reco.InclusiveKinematicsReconstructed_InclusiveKinematicsESigma, 'InclusiveKinematicsESigma',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsESigma'])
        app.add(meta.CollectionCollector_InclusiveKinematics, 'InclusiveKinematicseSigma_legacy',
                ['InclusiveKinematicsESigma'],
                ['InclusiveKinematicseSigma'])
        app.add(reco.InclusiveKinematicsReconstructed_InclusiveKinematicsSigma, 'InclusiveKinematicsSigma',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsSigma'])
        app.add(reco.InclusiveKinematicsML, 'InclusiveKinematicsML',
                ['InclusiveKinematicsElectron', 'InclusiveKinematicsDA'],
                ['InclusiveKinematicsML'])
        app.add(reco.ReconstructedElectrons, 'ReconstructedElectrons',
                ['ReconstructedParticles'],
                ['ReconstructedElectrons'])
        app.add(reco.ReconstructedElectrons, 'ReconstructedElectronsForDIS',
                ['ReconstructedParticles'],
                ['ReconstructedElectronsForDIS'])
        app.add(reco.JetReconstruction_ReconstructedParticle, 'GeneratedJets',
                ['EventHeader', 'GeneratedParticles'],
                ['GeneratedJets'])
        app.add(reco.JetReconstruction_ReconstructedParticle, 'ReconstructedJets',
                ['EventHeader', 'ReconstructedParticles'],
                ['ReconstructedJets'])
        app.add(reco.ChargedReconstructedParticleSelector, 'GeneratedChargedParticles',
                ['GeneratedParticles'],
                ['GeneratedChargedParticles'])
        app.add(reco.ClustersToParticles, 'ReconstructedNeutralParticles',
                ['EcalClusters', 'EcalClusterAssociations'],
                ['ReconstructedNeutralParticles', 'ReconstructedNeutralParticleLinks', 'ReconstructedNeutralParticleAssociations'])
        app.add(reco.JetReconstruction_ReconstructedParticle, 'GeneratedChargedJets',
                ['EventHeader', 'GeneratedChargedParticles'],
                ['GeneratedChargedJets'])
        app.add(reco.JetReconstruction_ReconstructedParticle, 'ReconstructedChargedJets',
                ['EventHeader', 'ReconstructedChargedParticles'],
                ['ReconstructedChargedJets'])
        app.add(reco.JetReconstruction_ReconstructedParticle, 'ReconstructedNeutralJets',
                ['EventHeader', 'ReconstructedNeutralParticles'],
                ['ReconstructedNeutralJets'])
        app.add(reco.ScatteredElectronsEMinusPz, 'ScatteredElectronsEMinusPz',
                ['ReconstructedChargedParticles', 'ReconstructedElectronsForDIS'],
                ['ScatteredElectronsEMinusPz'],
                maxEMinusPz=10000000.0)
        app.add(meta.SortSubsetCollection_ReconstructedParticle_reco_particle_pt, 'ScatteredElectronsEMinusPzByPt',
                ['ScatteredElectronsEMinusPz'],
                ['ScatteredElectronsEMinusPzByPt'])
        app.add(reco.TrackClusterMatch, 'EcalEndcapPTrackClusterMatches',
                ['CalorimeterTrackProjections', 'EcalEndcapPClusters'],
                ['EcalEndcapPTrackClusterMatches'],
                calo_id="EcalEndcapP_ID")
        app.add(reco.TrackClusterMatch, 'LFHCALTrackClusterMatches',
                ['CalorimeterTrackProjections', 'LFHCALClusters'],
                ['LFHCALTrackClusterMatches'],
                calo_id="LFHCAL_ID")
        app.add(reco.TrackClusterMatch, 'HcalEndcapPInsertClusterMatches',
                ['CalorimeterTrackProjections', 'HcalEndcapPInsertClusters'],
                ['HcalEndcapPInsertTrackClusterMatches'],
                calo_id="HcalEndcapPInsert_ID")
        app.add(reco.TrackClusterMatch, 'EcalBarrelTrackClusterMatches',
                ['CalorimeterTrackProjections', 'EcalBarrelClusters'],
                ['EcalBarrelTrackClusterMatches'],
                calo_id="EcalBarrel_ID")
        app.add(reco.TrackClusterMatch, 'HcalBarrelTrackClusterMatches',
                ['CalorimeterTrackProjections', 'HcalBarrelClusters'],
                ['HcalBarrelTrackClusterMatches'],
                calo_id="HcalBarrel_ID")
        app.add(reco.TrackClusterMatch, 'EcalEndcapNTrackClusterMatches',
                ['CalorimeterTrackProjections', 'EcalEndcapNClusters'],
                ['EcalEndcapNTrackClusterMatches'],
                calo_id="EcalEndcapN_ID")
        app.add(reco.TrackClusterMatch, 'HcalEndcapNTrackClusterMatches',
                ['CalorimeterTrackProjections', 'HcalEndcapNClusters'],
                ['HcalEndcapNTrackClusterMatches'],
                calo_id="HcalEndcapN_ID")
        app.add(reco.TransformBreitFrame, 'ReconstructedBreitFrameParticles',
                ['MCParticles', 'InclusiveKinematicsElectron', 'ReconstructedParticles'],
                ['ReconstructedBreitFrameParticles'])
        app.add(reco.FarForwardNeutralsReconstruction, 'ReconstructedFarForwardNeutrals',
                ['HcalFarForwardZDCClusters', 'B0ECalClusters', 'EcalEndcapPClusters', 'LFHCALClusters'],
                ['ReconstructedHcalFarForwardZDCNeutrals', 'ReconstructedB0EcalNeutrals', 'ReconstructedEcalEndcapPNeutrals', 'ReconstructedLFHCALNeutrals'])
        app.add(reco.LambdaReconstruction, 'ReconstructedLambdaCandidates',
                ['ReconstructedHcalFarForwardZDCNeutrals', 'ReconstructedB0EcalNeutrals', 'ReconstructedEcalEndcapPNeutrals', 'ReconstructedLFHCALNeutrals'],
                ['ReconstructedLambdaCandidates', 'ReconstructedLambdaCandidateDecayProductsCM'])
        app.add(reco.ChargedLambdaReconstruction, 'ReconstructedChargedLambdas',
                ['ReconstructedChargedParticles', 'ForwardRomanPotRecLabFrameParticles', 'ForwardOffMRecLabFrameParticles'],
                ['ReconstructedChargedLambdas'])
        app.add(reco.ChargedLambdaReconstruction, 'ReconstructedTruthSeededChargedLambdas',
                ['ReconstructedTruthSeededChargedParticles', 'ForwardRomanPotRecLabFrameParticles', 'ForwardOffMRecLabFrameParticles'],
                ['ReconstructedTruthSeededChargedLambdas'])
        app.add(reco.TransformBreitFrame, 'GeneratedBreitFrameParticles',
                ['MCParticles', 'InclusiveKinematicsElectron', 'GeneratedParticles'],
                ['GeneratedBreitFrameParticles'])
        app.add(reco.JetReconstruction_ReconstructedParticle, 'GeneratedCentauroJets',
                ['EventHeader', 'GeneratedBreitFrameParticles'],
                ['GeneratedCentauroJets'],
                rJet=0.8,
                jetAlgo="plugin_algorithm")
        app.add(reco.JetReconstruction_ReconstructedParticle, 'ReconstructedCentauroJets',
                ['EventHeader', 'ReconstructedBreitFrameParticles'],
                ['ReconstructedCentauroJets'],
                rJet=0.8,
                jetAlgo="plugin_algorithm")
        app.add(reco.UndoAfterBurnerMCParticles, 'MCParticlesHeadOnFrameNoBeamFX',
                ['MCParticles'],
                ['MCParticlesHeadOnFrameNoBeamFX'])
        app.add(reco.PrimaryVertices, 'PrimaryVertices',
                ['CentralTrackVertices'],
                ['PrimaryVertices'])
        app.add(reco.SecondaryVerticesHelix, 'SecondaryVerticesHelix',
                ['PrimaryVertices', 'ReconstructedParticles'],
                ['SecondaryVerticesHelix'])
        app.add(reco.Truthiness, 'Truthiness',
                ['MCParticles', 'ReconstructedParticles', 'ReconstructedParticleAssociations'],
                ['Truthiness'])
