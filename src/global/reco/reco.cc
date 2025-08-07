// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025, Dmitry Romanov, Nathan Brei, Tooba Ali, Wouter Deconinck, Dmitry Kalinkin, John Lajoie, Simon Gardner, Tristan Protzman, Daniel Brandenburg, Derek M Anderson, Sebouh Paul, Tyler Kutz, Alex Jentsch, Jihee Kim, Brian Page

#include <Evaluator/DD4hepUnits.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/InclusiveKinematics.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4hep/MCParticle.h>
#include <fmt/core.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/reco/HadronicFinalState.h"
#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsESigma.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/FilterMatching_factory.h"
#include "factories/reco/FarForwardLambdaReconstruction_factory.h"
#include "factories/reco/FarForwardNeutralsReconstruction_factory.h"
#ifdef USE_ONNX
#include "factories/reco/InclusiveKinematicsML_factory.h"
#endif
#include "factories/reco/ChargedReconstructedParticleSelector_factory.h"
#include "factories/reco/HadronicFinalState_factory.h"
#include "factories/reco/InclusiveKinematicsReconstructed_factory.h"
#include "factories/reco/InclusiveKinematicsTruth_factory.h"
#include "factories/reco/JetReconstruction_factory.h"
#include "factories/reco/MC2ReconstructedParticle_factory.h"
#include "factories/reco/MatchClusters_factory.h"
#include "factories/reco/PrimaryVertices_factory.h"
#include "factories/reco/ReconstructedElectrons_factory.h"
#include "factories/reco/ScatteredElectronsEMinusPz_factory.h"
#include "factories/reco/ScatteredElectronsTruth_factory.h"
#include "factories/reco/TransformBreitFrame_factory.h"
#include "factories/reco/UndoAfterBurnerMCParticles_factory.h"

#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/reco/TrackClusterMatch_factory.h"
#endif

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using jana::components::JOmniFactoryGeneratorT;

  // Finds associations matched to initial scattered electrons
  app->Add(
      new JOmniFactoryGeneratorT<FilterMatching_factory<
          edm4eic::MCRecoParticleAssociation, [](auto* obj) { return obj->getSim().getObjectID(); },
          edm4hep::MCParticle, [](auto* obj) { return obj->getObjectID(); }>>(
          "MCScatteredElectronAssociations",
          {"ReconstructedChargedParticleAssociations", "MCScatteredElectrons"},
          {"MCScatteredElectronAssociations", "MCNonScatteredElectronAssociations"}));

  app->Add(new JOmniFactoryGeneratorT<MC2ReconstructedParticle_factory>(
      "GeneratedParticles", {"MCParticles"}, {"GeneratedParticles"}));

#if (10000 * JANA_VERSION_MAJOR + 100 * JANA_VERSION_MINOR + JANA_VERSION_PATCH) < 20403
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster, true>>(
      "EcalClusters", {"EcalEndcapNClusters", "EcalBarrelScFiClusters", "EcalEndcapPClusters"},
      {"EcalClusters"}));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoClusterParticleAssociation, true>>(
      "EcalClusterAssociations",
      {"EcalEndcapNClusterAssociations", "EcalBarrelScFiClusterAssociations",
       "EcalEndcapPClusterAssociations"},
      {"EcalClusterAssociations"}));
#else
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster, true>>(
      {.tag                  = "EcalClusters",
       .variadic_input_names = {{"EcalEndcapNClusters", "EcalBarrelScFiClusters",
                                 "EcalEndcapPClusters"}},
       .output_names         = {"EcalClusters"}}));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoClusterParticleAssociation, true>>(
      {.tag                  = "EcalClusterAssociations",
       .variadic_input_names = {{"EcalEndcapNClusterAssociations",
                                 "EcalBarrelScFiClusterAssociations",
                                 "EcalEndcapPClusterAssociations"}},
       .output_names         = {"EcalClusterAssociations"}}));
#endif

  app->Add(new JOmniFactoryGeneratorT<MatchClusters_factory>(
      "ReconstructedParticlesWithAssoc",
      {
          "MCParticles",
          "ReconstructedChargedParticles",
          "ReconstructedChargedParticleAssociations",
          "EcalClusters",
          "EcalClusterAssociations",
      },
      {
          "ReconstructedParticles",           // edm4eic::ReconstructedParticle
          "ReconstructedParticleAssociations" // edm4eic::MCRecoParticleAssociation
      }));

  app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsTruth_factory>(
      "InclusiveKinematicsTruth", {"MCParticles"}, {"InclusiveKinematicsTruth"}));

  app->Add(new JOmniFactoryGeneratorT<
           InclusiveKinematicsReconstructed_factory<InclusiveKinematicsElectron>>(
      "InclusiveKinematicsElectron",
      {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
      {"InclusiveKinematicsElectron"}));

  app->Add(
      new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsJB>>(
          "InclusiveKinematicsJB", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
          {"InclusiveKinematicsJB"}));

  app->Add(
      new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsDA>>(
          "InclusiveKinematicsDA", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
          {"InclusiveKinematicsDA"}));

  app->Add(new JOmniFactoryGeneratorT<
           InclusiveKinematicsReconstructed_factory<InclusiveKinematicsESigma>>(
      "InclusiveKinematicsESigma", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
      {"InclusiveKinematicsESigma"}));

#if (10000 * JANA_VERSION_MAJOR + 100 * JANA_VERSION_MINOR + JANA_VERSION_PATCH) < 20403
  // InclusiveKinematicseSigma is deprecated and will be removed, use InclusiveKinematicsESigma instead
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::InclusiveKinematics>>(
      "InclusiveKinematicseSigma_legacy", {"InclusiveKinematicsESigma"},
      {"InclusiveKinematicseSigma"}));
#else
  // InclusiveKinematicseSigma is deprecated and will be removed, use InclusiveKinematicsESigma instead
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::InclusiveKinematics>>(
      {.tag                  = "InclusiveKinematicseSigma_legacy",
       .variadic_input_names = {{"InclusiveKinematicsESigma"}},
       .output_names         = {"InclusiveKinematicseSigma"}}));
#endif

  app->Add(new JOmniFactoryGeneratorT<
           InclusiveKinematicsReconstructed_factory<InclusiveKinematicsSigma>>(
      "InclusiveKinematicsSigma", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
      {"InclusiveKinematicsSigma"}));

#ifdef USE_ONNX
  app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsML_factory>(
      "InclusiveKinematicsML", {"InclusiveKinematicsElectron", "InclusiveKinematicsDA"},
      {"InclusiveKinematicsML"}));
#endif

  app->Add(new JOmniFactoryGeneratorT<ReconstructedElectrons_factory>(
      "ReconstructedElectrons", {"ReconstructedParticles"}, {"ReconstructedElectrons"}, {}));

  app->Add(new JOmniFactoryGeneratorT<ReconstructedElectrons_factory>(
      "ReconstructedElectronsForDIS", {"ReconstructedParticles"}, {"ReconstructedElectronsForDIS"},
      {
          .min_energy_over_momentum = 0.7, // GeV
          .max_energy_over_momentum = 1.3  // GeV
      }));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "GeneratedJets", {"GeneratedParticles"}, {"GeneratedJets"}, {}));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "ReconstructedJets", {"ReconstructedParticles"}, {"ReconstructedJets"}, {}));

  app->Add(new JOmniFactoryGeneratorT<ChargedReconstructedParticleSelector_factory>(
      "GeneratedChargedParticles", {"GeneratedParticles"}, {"GeneratedChargedParticles"}));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "GeneratedChargedJets", {"GeneratedChargedParticles"}, {"GeneratedChargedJets"}, {}));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "ReconstructedChargedJets", {"ReconstructedChargedParticles"}, {"ReconstructedChargedJets"},
      {}));

  app->Add(new JOmniFactoryGeneratorT<ScatteredElectronsTruth_factory>(
      "ScatteredElectronsTruth",
      {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"},
      {"ScatteredElectronsTruth"}));

  app->Add(new JOmniFactoryGeneratorT<ScatteredElectronsEMinusPz_factory>(
      "ScatteredElectronsEMinusPz",
      {"ReconstructedChargedParticles", "ReconstructedElectronsForDIS"},
      {"ScatteredElectronsEMinusPz"},
      {
          .minEMinusPz = 0,         // GeV
          .maxEMinusPz = 10000000.0 // GeV
      }));

#if EDM4EIC_VERSION_MAJOR >= 8
  // Forward
  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "EcalEndcapPTrackClusterMatches", {"CalorimeterTrackProjections", "EcalEndcapPClusters"},
      {"EcalEndcapPTrackClusterMatches"}, {.calo_id = "EcalEndcapP_ID"}));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "LFHCALTrackClusterMatches", {"CalorimeterTrackProjections", "LFHCALClusters"},
      {"LFHCALTrackClusterMatches"}, {.calo_id = "LFHCAL_ID"}));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "HcalEndcapPInsertClusterMatches",
      {"CalorimeterTrackProjections", "HcalEndcapPInsertClusters"},
      {"HcalEndcapPInsertTrackClusterMatches"}, {.calo_id = "HcalEndcapPInsert_ID"}));

  // Barrel
  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "EcalBarrelTrackClusterMatches", {"CalorimeterTrackProjections", "EcalBarrelClusters"},
      {"EcalBarrelTrackClusterMatches"}, {.calo_id = "EcalBarrel_ID"}));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "HcalBarrelTrackClusterMatches", {"CalorimeterTrackProjections", "HcalBarrelClusters"},
      {"HcalBarrelTrackClusterMatches"}, {.calo_id = "HcalBarrel_ID"}));

  // Backward
  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "EcalEndcapNBarrelTrackClusterMatches",
      {"CalorimeterTrackProjections", "EcalEndcapNClusters"}, {"EcalEndcapNTrackClusterMatches"},
      {.calo_id = "EcalEndcapN_ID"}));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "HcalEndcapNBarrelTrackClusterMatches",
      {"CalorimeterTrackProjections", "HcalEndcapNClusters"}, {"HcalEndcapNTrackClusterMatches"},
      {.calo_id = "HcalEndcapN_ID"}));

#endif // EDM4EIC_VERSION_MAJOR >= 8

  app->Add(new JOmniFactoryGeneratorT<TransformBreitFrame_factory>(
      "ReconstructedBreitFrameParticles",
      {"MCParticles", "InclusiveKinematicsElectron", "ReconstructedParticles"},
      {"ReconstructedBreitFrameParticles"}, {}));

  app->Add(new JOmniFactoryGeneratorT<FarForwardNeutralsReconstruction_factory>(
      "ReconstructedFarForwardZDCNeutrons",
      {"HcalFarForwardZDCClusters"},          // edm4eic::ClusterCollection
      {"ReconstructedFarForwardZDCNeutrals"}, // edm4eic::ReconstrutedParticleCollection,
      {.offsetPositionName        = "HcalFarForwardZDC_SiPMonTile_r_pos",
       .neutronScaleCorrCoeffHcal = {-0.11, -1.5, 0},
       .gammaScaleCorrCoeffHcal   = {0, -.13, 0},
       .globalToProtonRotation    = -0.025,
       .gammaZMaxOffset           = 300 * dd4hep::mm,
       .gammaMaxLength            = 100 * dd4hep::mm,
       .gammaMaxWidth             = 12 * dd4hep::mm}));

  app->Add(new JOmniFactoryGeneratorT<FarForwardLambdaReconstruction_factory>(
      "ReconstructedFarForwardZDCLambdas",
      {"ReconstructedFarForwardZDCNeutrals"}, // edm4eic::ReconstrutedParticleCollection,
      {"ReconstructedFarForwardZDCLambdas", "ReconstructedFarForwardZDCLambdaDecayProductsC"
                                            "M"}, // edm4eic::ReconstrutedParticleCollection,
      {.offsetPositionName     = "HcalFarForwardZDC_SiPMonTile_r_pos",
       .globalToProtonRotation = -0.025,
       .lambdaMaxMassDev       = 0.030 * dd4hep::GeV,
       .iterations             = 10}));

  app->Add(new JOmniFactoryGeneratorT<HadronicFinalState_factory<HadronicFinalState>>(
      "HadronicFinalState",
      {"MCParticles", "ReconstructedParticles", "ReconstructedParticleAssociations"},
      {"HadronicFinalState"}));

  app->Add(new JOmniFactoryGeneratorT<TransformBreitFrame_factory>(
      "GeneratedBreitFrameParticles",
      {"MCParticles", "InclusiveKinematicsElectron", "GeneratedParticles"},
      {"GeneratedBreitFrameParticles"}, {}));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "GeneratedCentauroJets", {"GeneratedBreitFrameParticles"}, {"GeneratedCentauroJets"},
      {.rJet = 0.8, .jetAlgo = "plugin_algorithm", .jetContribAlgo = "Centauro"}));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "ReconstructedCentauroJets", {"ReconstructedBreitFrameParticles"},
      {"ReconstructedCentauroJets"},
      {.rJet = 0.8, .jetAlgo = "plugin_algorithm", .jetContribAlgo = "Centauro"}));

  //Full correction for MCParticles --> MCParticlesHeadOnFrame
  app->Add(new JOmniFactoryGeneratorT<UndoAfterBurnerMCParticles_factory>(
      "MCParticlesHeadOnFrameNoBeamFX", {"MCParticles"}, {"MCParticlesHeadOnFrameNoBeamFX"},
      {
          .m_pid_assume_pion_mass = false,
          .m_crossing_angle       = -0.025 * dd4hep::rad,
          .m_pid_purity           = 0.51, //dummy value for MC truth information
          .m_correct_beam_FX      = true,
          .m_pid_use_MC_truth     = true,
      }));

  app->Add(new JOmniFactoryGeneratorT<PrimaryVertices_factory>(
      "PrimaryVertices", {"CentralTrackVertices"}, {"PrimaryVertices"}, {}));
}
} // extern "C"
