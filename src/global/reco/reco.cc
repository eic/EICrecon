// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025, Dmitry Romanov, Nathan Brei, Tooba Ali, Wouter Deconinck, Dmitry Kalinkin, John Lajoie, Simon Gardner, Tristan Protzman, Daniel Brandenburg, Derek M Anderson, Sebouh Paul, Tyler Kutz, Alex Jentsch, Jihee Kim, Brian Page

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <algorithm>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/InclusiveKinematics.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4hep/MCParticle.h>
#include <fmt/core.h>
#include <gsl/pointers>

#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>

#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_VERSION_MAJOR >= 6
#include "algorithms/reco/HadronicFinalState.h"
#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsESigma.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#endif
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/FilterMatching_factory.h"
#include "factories/reco/FarForwardLambdaReconstruction_factory.h"
#include "factories/reco/FarForwardNeutralsReconstruction_factory.h"
#ifdef USE_ONNX
#include "factories/reco/InclusiveKinematicsML_factory.h"
#endif
#if EDM4EIC_VERSION_MAJOR >= 6
#include "factories/reco/InclusiveKinematicsReconstructed_factory.h"
#endif
#include "factories/reco/InclusiveKinematicsTruth_factory.h"
#include "factories/reco/JetReconstruction_factory.h"
#include "factories/reco/TransformBreitFrame_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 6
#include "factories/reco/HadronicFinalState_factory.h"
#endif
#include "factories/reco/UndoAfterBurnerMCParticles_factory.h"
#include "factories/reco/ChargedReconstructedParticleSelector_factory.h"
#include "factories/reco/MC2SmearedParticle_factory.h"
#include "factories/reco/MatchClusters_factory.h"
#include "factories/reco/PrimaryVertices_factory.h"
#include "factories/reco/ReconstructedElectrons_factory.h"
#include "factories/reco/ScatteredElectronsEMinusPz_factory.h"
#include "factories/reco/ScatteredElectronsTruth_factory.h"

#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/reco/TrackClusterMatch_factory.h"
#endif

#include "services/geometry/dd4hep/DD4hep_service.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Finds associations matched to initial scattered electrons
  app->Add(
      new JOmniFactoryGeneratorT<FilterMatching_factory<
          edm4eic::MCRecoParticleAssociation, [](auto* obj) { return obj->getSim().getObjectID(); },
          edm4hep::MCParticle, [](auto* obj) { return obj->getObjectID(); }>>(
          "MCScatteredElectronAssociations",
          {"ReconstructedChargedParticleAssociations", "MCScatteredElectrons"},
          {"MCScatteredElectronAssociations", "MCNonScatteredElectronAssociations"}, app));

  app->Add(new JOmniFactoryGeneratorT<MC2SmearedParticle_factory>(
      "GeneratedParticles", {"MCParticles"}, {"GeneratedParticles"}, app));

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster, true>>(
      "EcalClusters", {"EcalEndcapNClusters", "EcalBarrelScFiClusters", "EcalEndcapPClusters"},
      {"EcalClusters"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoClusterParticleAssociation, true>>(
      "EcalClusterAssociations",
      {"EcalEndcapNClusterAssociations", "EcalBarrelScFiClusterAssociations",
       "EcalEndcapPClusterAssociations"},
      {"EcalClusterAssociations"}, app));

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
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsTruth_factory>(
      "InclusiveKinematicsTruth", {"MCParticles"}, {"InclusiveKinematicsTruth"}, app));

#if EDM4EIC_VERSION_MAJOR >= 6
  app->Add(new JOmniFactoryGeneratorT<
           InclusiveKinematicsReconstructed_factory<InclusiveKinematicsElectron>>(
      "InclusiveKinematicsElectron",
      {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
      {"InclusiveKinematicsElectron"}, app));

  app->Add(
      new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsJB>>(
          "InclusiveKinematicsJB", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
          {"InclusiveKinematicsJB"}, app));

  app->Add(
      new JOmniFactoryGeneratorT<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsDA>>(
          "InclusiveKinematicsDA", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
          {"InclusiveKinematicsDA"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           InclusiveKinematicsReconstructed_factory<InclusiveKinematicsESigma>>(
      "InclusiveKinematicsESigma", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
      {"InclusiveKinematicsESigma"}, app));

  // InclusiveKinematicseSigma is deprecated and will be removed, use InclusiveKinematicsESigma instead
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::InclusiveKinematics>>(
      "InclusiveKinematicseSigma_legacy", {"InclusiveKinematicsESigma"},
      {"InclusiveKinematicseSigma"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           InclusiveKinematicsReconstructed_factory<InclusiveKinematicsSigma>>(
      "InclusiveKinematicsSigma", {"MCParticles", "ScatteredElectronsTruth", "HadronicFinalState"},
      {"InclusiveKinematicsSigma"}, app));

#ifdef USE_ONNX
  app->Add(new JOmniFactoryGeneratorT<InclusiveKinematicsML_factory>(
      "InclusiveKinematicsML", {"InclusiveKinematicsElectron", "InclusiveKinematicsDA"},
      {"InclusiveKinematicsML"}, app));
#endif
#endif

  app->Add(new JOmniFactoryGeneratorT<ReconstructedElectrons_factory>(
      "ReconstructedElectrons", {"ReconstructedParticles"}, {"ReconstructedElectrons"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<ReconstructedElectrons_factory>(
      "ReconstructedElectronsForDIS", {"ReconstructedParticles"}, {"ReconstructedElectronsForDIS"},
      {
          .min_energy_over_momentum = 0.7, // GeV
          .max_energy_over_momentum = 1.3  // GeV
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "GeneratedJets", {"GeneratedParticles"}, {"GeneratedJets"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "ReconstructedJets", {"ReconstructedParticles"}, {"ReconstructedJets"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<ChargedReconstructedParticleSelector_factory>(
      "GeneratedChargedParticles", {"GeneratedParticles"}, {"GeneratedChargedParticles"}, app));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "GeneratedChargedJets", {"GeneratedChargedParticles"}, {"GeneratedChargedJets"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "ReconstructedChargedJets", {"ReconstructedChargedParticles"}, {"ReconstructedChargedJets"},
      {}, app));

  app->Add(new JOmniFactoryGeneratorT<ScatteredElectronsTruth_factory>(
      "ScatteredElectronsTruth",
      {"MCParticles", "ReconstructedChargedParticles", "ReconstructedChargedParticleAssociations"},
      {"ScatteredElectronsTruth"}, app));

  app->Add(new JOmniFactoryGeneratorT<ScatteredElectronsEMinusPz_factory>(
      "ScatteredElectronsEMinusPz",
      {"ReconstructedChargedParticles", "ReconstructedElectronsForDIS"},
      {"ScatteredElectronsEMinusPz"},
      {
          .minEMinusPz = 0,         // GeV
          .maxEMinusPz = 10000000.0 // GeV
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Cluster, true>>(
      "BarrelClusters",
      {
          "HcalBarrelClusters",
          "EcalBarrelClusters",
      },
      {"BarrelClusters"}, app));

#if EDM4EIC_VERSION_MAJOR >= 8
  app->Add(new JOmniFactoryGeneratorT<TrackClusterMatch_factory>(
      "TrackClusterMatcher", {"CalorimeterTrackProjections", "BarrelClusters"},
      {"TrackClusterMatches"}, {}, app));
#endif // EDM4EIC_VERSION_MAJOR >= 8

  app->Add(new JOmniFactoryGeneratorT<TransformBreitFrame_factory>(
      "ReconstructedBreitFrameParticles",
      {"MCParticles", "InclusiveKinematicsElectron", "ReconstructedParticles"},
      {"ReconstructedBreitFrameParticles"}, {}, app));

  auto detector = app->GetService<DD4hep_service>()->detector();
  double z_zdc  = NAN;
  try {
    z_zdc = detector->constant<double>("HcalFarForwardZDC_SiPMonTile_r_pos") / dd4hep::mm;
  } catch (std::runtime_error&) {
    z_zdc = 35800;
  }

  app->Add(new JOmniFactoryGeneratorT<FarForwardNeutralsReconstruction_factory>(
      "ReconstructedFarForwardZDCNeutrons",
      {"HcalFarForwardZDCClusters"},          // edm4eic::ClusterCollection
      {"ReconstructedFarForwardZDCNeutrals"}, // edm4eic::ReconstrutedParticleCollection,
      {.neutronScaleCorrCoeffHcal = {-0.11, -1.5, 0},
       .gammaScaleCorrCoeffHcal   = {0, -.13, 0},
       .globalToProtonRotation    = -0.025,
       .gammaZMax                 = (300 + z_zdc) * dd4hep::mm,
       .gammaMaxLength            = 100 * dd4hep::mm,
       .gammaMaxWidth             = 12 * dd4hep::mm},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<FarForwardLambdaReconstruction_factory>(
      "ReconstructedFarForwardZDCLambdas",
      {"ReconstructedFarForwardZDCNeutrals"}, // edm4eic::ReconstrutedParticleCollection,
      {"ReconstructedFarForwardZDCLambdas", "ReconstructedFarForwardZDCLambdaDecayProductsC"
                                            "M"}, // edm4eic::ReconstrutedParticleCollection,
      {.globalToProtonRotation = -0.025,
       .zMax                   = z_zdc * dd4hep::mm,
       .lambdaMaxMassDev       = 0.030 * dd4hep::GeV,
       .iterations             = 10},
      app // TODO: Remove me once fixed
      ));

#if EDM4EIC_VERSION_MAJOR >= 6
  app->Add(new JOmniFactoryGeneratorT<HadronicFinalState_factory<HadronicFinalState>>(
      "HadronicFinalState",
      {"MCParticles", "ReconstructedParticles", "ReconstructedParticleAssociations"},
      {"HadronicFinalState"}, app));
#endif

  app->Add(new JOmniFactoryGeneratorT<TransformBreitFrame_factory>(
      "GeneratedBreitFrameParticles",
      {"MCParticles", "InclusiveKinematicsElectron", "GeneratedParticles"},
      {"GeneratedBreitFrameParticles"}, {}, app));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "GeneratedCentauroJets", {"GeneratedBreitFrameParticles"}, {"GeneratedCentauroJets"},
      {.rJet = 0.8, .jetAlgo = "plugin_algorithm", .jetContribAlgo = "Centauro"}, app));

  app->Add(new JOmniFactoryGeneratorT<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      "ReconstructedCentauroJets", {"ReconstructedBreitFrameParticles"},
      {"ReconstructedCentauroJets"},
      {.rJet = 0.8, .jetAlgo = "plugin_algorithm", .jetContribAlgo = "Centauro"}, app));

  //Full correction for MCParticles --> MCParticlesHeadOnFrame
  app->Add(new JOmniFactoryGeneratorT<UndoAfterBurnerMCParticles_factory>(
      "MCParticlesHeadOnFrameNoBeamFX", {"MCParticles"}, {"MCParticlesHeadOnFrameNoBeamFX"},
      {
          .m_pid_assume_pion_mass = false,
          .m_crossing_angle       = -0.025 * dd4hep::rad,
          .m_pid_purity           = 0.51, //dummy value for MC truth information
          .m_correct_beam_FX      = true,
          .m_pid_use_MC_truth     = true,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<PrimaryVertices_factory>(
      "PrimaryVertices", {"CentralTrackVertices"}, {"PrimaryVertices"}, {}, app));
}
} // extern "C"
