# SPDX-License-Identifier: LGPL-3.0-or-later
"""reco wiring, converted from src/global/reco/reco.cc by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "reco"


def configure(app, plugin=PLUGIN):
    T = bindings("reco")
    with app.plugin(plugin):
        app.add(T.FilterMatching_MCRecoParticleLink_to_MCParticle_self, 'MCScatteredElectronLinks',
                ['ReconstructedChargedParticleLinks', 'MCScatteredElectrons'],
                ['MCScatteredElectronLinks', 'MCNonScatteredElectronLinks'])
        app.add(T.FilterMatching_MCRecoParticleAssociation_sim_MCParticle_self, 'MCScatteredElectronAssociations',
                ['ReconstructedChargedParticleAssociations', 'MCScatteredElectrons'],
                ['MCScatteredElectronAssociations', 'MCNonScatteredElectronAssociations'])
        app.add(T.MC2ReconstructedParticle, 'GeneratedParticles',
                ['MCParticles'],
                ['GeneratedParticles'])
        app.add(T.CollectionCollector_Cluster_true, 'EcalClusters',
                ['EcalEndcapNClusters', 'EcalBarrelClusters', 'EcalEndcapPClusters'],
                ['EcalClusters'])
        app.add(T.CollectionCollector_MCRecoClusterParticleAssociation_true, 'EcalClusterAssociations',
                ['EcalEndcapNClusterAssociations', 'EcalBarrelClusterAssociations', 'EcalEndcapPClusterAssociations'],
                ['EcalClusterAssociations'])
        app.add(T.MatchClusters, 'ReconstructedParticlesWithAssoc',
                ['MCParticles', 'ReconstructedChargedParticles', 'ReconstructedChargedParticleAssociations', 'EcalClusters', 'EcalClusterAssociations'],
                ['ReconstructedParticles', 'ReconstructedParticleLinks', 'ReconstructedParticleAssociations'])
        app.add(T.InclusiveKinematicsTruthBare, 'InclusiveKinematicsTruthBare',
                ['MCParticles'],
                ['InclusiveKinematicsTruthBare'])
        app.add(T.CollectionCollector_InclusiveKinematics, 'InclusiveKinematicsTruth',
                ['InclusiveKinematicsTruthBare'],
                ['InclusiveKinematicsTruth'])
        app.add(T.InclusiveKinematicsTruthBorn, 'InclusiveKinematicsTruthBorn',
                ['MCParticles'],
                ['InclusiveKinematicsTruthBorn'])
        app.add(T.InclusiveKinematicsTruthDressed, 'InclusiveKinematicsTruthDressed',
                ['MCParticles'],
                ['InclusiveKinematicsTruthDressed', 'ScatteredElectronTruthDressed', 'ScatteredElectronTruthDressedConstituents'])
        app.add(T.InclusiveKinematicsReconstructed_InclusiveKinematicsElectron, 'InclusiveKinematicsElectron',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsElectron'])
        app.add(T.ScatteredElectronsTruth, 'ScatteredElectronsTruth',
                ['MCParticles', 'ReconstructedChargedParticles', 'ReconstructedChargedParticleAssociations'],
                ['ScatteredElectronsTruth'])
        app.add(T.HadronicFinalState_HadronicFinalState, 'HadronicFinalState',
                ['MCBeamElectrons', 'MCBeamHadrons', 'MCParticles', 'ReconstructedParticles', 'ReconstructedParticleAssociations'],
                ['HadronicFinalState'])
        app.add(T.InclusiveKinematicsReconstructed_InclusiveKinematicsJB, 'InclusiveKinematicsJB',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsJB'])
        app.add(T.InclusiveKinematicsReconstructed_InclusiveKinematicsDA, 'InclusiveKinematicsDA',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsDA'])
        app.add(T.InclusiveKinematicsReconstructed_InclusiveKinematicsESigma, 'InclusiveKinematicsESigma',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsESigma'])
        app.add(T.CollectionCollector_InclusiveKinematics, 'InclusiveKinematicseSigma_legacy',
                ['InclusiveKinematicsESigma'],
                ['InclusiveKinematicseSigma'])
        app.add(T.InclusiveKinematicsReconstructed_InclusiveKinematicsSigma, 'InclusiveKinematicsSigma',
                ['MCBeamElectrons', 'MCBeamHadrons', 'ScatteredElectronsTruth', 'HadronicFinalState'],
                ['InclusiveKinematicsSigma'])
        app.add(T.InclusiveKinematicsML, 'InclusiveKinematicsML',
                ['InclusiveKinematicsElectron', 'InclusiveKinematicsDA'],
                ['InclusiveKinematicsML'])
        app.add(T.ReconstructedElectrons, 'ReconstructedElectrons',
                ['ReconstructedParticles'],
                ['ReconstructedElectrons'])
        app.add(T.ReconstructedElectrons, 'ReconstructedElectronsForDIS',
                ['ReconstructedParticles'],
                ['ReconstructedElectronsForDIS'])
        app.add(T.JetReconstruction_ReconstructedParticle, 'GeneratedJets',
                ['EventHeader', 'GeneratedParticles'],
                ['GeneratedJets'])
        app.add(T.JetReconstruction_ReconstructedParticle, 'ReconstructedJets',
                ['EventHeader', 'ReconstructedParticles'],
                ['ReconstructedJets'])
        app.add(T.ChargedReconstructedParticleSelector, 'GeneratedChargedParticles',
                ['GeneratedParticles'],
                ['GeneratedChargedParticles'])
        app.add(T.ClustersToParticles, 'ReconstructedNeutralParticles',
                ['EcalClusters', 'EcalClusterAssociations'],
                ['ReconstructedNeutralParticles', 'ReconstructedNeutralParticleLinks', 'ReconstructedNeutralParticleAssociations'])
        app.add(T.JetReconstruction_ReconstructedParticle, 'GeneratedChargedJets',
                ['EventHeader', 'GeneratedChargedParticles'],
                ['GeneratedChargedJets'])
        app.add(T.JetReconstruction_ReconstructedParticle, 'ReconstructedChargedJets',
                ['EventHeader', 'ReconstructedChargedParticles'],
                ['ReconstructedChargedJets'])
        app.add(T.JetReconstruction_ReconstructedParticle, 'ReconstructedNeutralJets',
                ['EventHeader', 'ReconstructedNeutralParticles'],
                ['ReconstructedNeutralJets'])
        app.add(T.ScatteredElectronsEMinusPz, 'ScatteredElectronsEMinusPz',
                ['ReconstructedChargedParticles', 'ReconstructedElectronsForDIS'],
                ['ScatteredElectronsEMinusPz'],
                maxEMinusPz=10000000.0)
        app.add(T.SortSubsetCollection_ReconstructedParticle_reco_particle_pt, 'ScatteredElectronsEMinusPzByPt',
                ['ScatteredElectronsEMinusPz'],
                ['ScatteredElectronsEMinusPzByPt'])
        app.add(T.TrackClusterMatch, 'EcalEndcapPTrackClusterMatches',
                ['CalorimeterTrackProjections', 'EcalEndcapPClusters'],
                ['EcalEndcapPTrackClusterMatches'],
                calo_id="EcalEndcapP_ID")
        app.add(T.TrackClusterMatch, 'LFHCALTrackClusterMatches',
                ['CalorimeterTrackProjections', 'LFHCALClusters'],
                ['LFHCALTrackClusterMatches'],
                calo_id="LFHCAL_ID")
        app.add(T.TrackClusterMatch, 'HcalEndcapPInsertClusterMatches',
                ['CalorimeterTrackProjections', 'HcalEndcapPInsertClusters'],
                ['HcalEndcapPInsertTrackClusterMatches'],
                calo_id="HcalEndcapPInsert_ID")
        app.add(T.TrackClusterMatch, 'EcalBarrelTrackClusterMatches',
                ['CalorimeterTrackProjections', 'EcalBarrelClusters'],
                ['EcalBarrelTrackClusterMatches'],
                calo_id="EcalBarrel_ID")
        app.add(T.TrackClusterMatch, 'HcalBarrelTrackClusterMatches',
                ['CalorimeterTrackProjections', 'HcalBarrelClusters'],
                ['HcalBarrelTrackClusterMatches'],
                calo_id="HcalBarrel_ID")
        app.add(T.TrackClusterMatch, 'EcalEndcapNTrackClusterMatches',
                ['CalorimeterTrackProjections', 'EcalEndcapNClusters'],
                ['EcalEndcapNTrackClusterMatches'],
                calo_id="EcalEndcapN_ID")
        app.add(T.TrackClusterMatch, 'HcalEndcapNTrackClusterMatches',
                ['CalorimeterTrackProjections', 'HcalEndcapNClusters'],
                ['HcalEndcapNTrackClusterMatches'],
                calo_id="HcalEndcapN_ID")
        app.add(T.TransformBreitFrame, 'ReconstructedBreitFrameParticles',
                ['MCParticles', 'InclusiveKinematicsElectron', 'ReconstructedParticles'],
                ['ReconstructedBreitFrameParticles'])
        app.add(T.FarForwardNeutralsReconstruction, 'ReconstructedFarForwardNeutrals',
                ['HcalFarForwardZDCClusters', 'B0ECalClusters', 'EcalEndcapPClusters', 'LFHCALClusters'],
                ['ReconstructedHcalFarForwardZDCNeutrals', 'ReconstructedB0EcalNeutrals', 'ReconstructedEcalEndcapPNeutrals', 'ReconstructedLFHCALNeutrals'])
        app.add(T.LambdaReconstruction, 'ReconstructedLambdaCandidates',
                ['ReconstructedHcalFarForwardZDCNeutrals', 'ReconstructedB0EcalNeutrals', 'ReconstructedEcalEndcapPNeutrals', 'ReconstructedLFHCALNeutrals'],
                ['ReconstructedLambdaCandidates', 'ReconstructedLambdaCandidateDecayProductsCM'])
        app.add(T.TransformBreitFrame, 'GeneratedBreitFrameParticles',
                ['MCParticles', 'InclusiveKinematicsElectron', 'GeneratedParticles'],
                ['GeneratedBreitFrameParticles'])
        app.add(T.JetReconstruction_ReconstructedParticle, 'GeneratedCentauroJets',
                ['EventHeader', 'GeneratedBreitFrameParticles'],
                ['GeneratedCentauroJets'],
                rJet=0.8,
                jetAlgo="plugin_algorithm")
        app.add(T.JetReconstruction_ReconstructedParticle, 'ReconstructedCentauroJets',
                ['EventHeader', 'ReconstructedBreitFrameParticles'],
                ['ReconstructedCentauroJets'],
                rJet=0.8,
                jetAlgo="plugin_algorithm")
        app.add(T.UndoAfterBurnerMCParticles, 'MCParticlesHeadOnFrameNoBeamFX',
                ['MCParticles'],
                ['MCParticlesHeadOnFrameNoBeamFX'])
        app.add(T.PrimaryVertices, 'PrimaryVertices',
                ['CentralTrackVertices'],
                ['PrimaryVertices'])
        app.add(T.SecondaryVerticesHelix, 'SecondaryVerticesHelix',
                ['PrimaryVertices', 'ReconstructedParticles'],
                ['SecondaryVerticesHelix'])
        app.add(T.Truthiness, 'Truthiness',
                ['MCParticles', 'ReconstructedParticles', 'ReconstructedParticleAssociations'],
                ['Truthiness'])
