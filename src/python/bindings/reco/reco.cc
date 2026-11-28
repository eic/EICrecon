// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._reco: the reconstruction factory types
// (src/factories/reco) for eicrecon.factories.reco, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "SubDivideFunctorBindings.h"
#include "algorithms/reco/HadronicFinalState.h"
#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsESigma.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#include "factories/meta/ObjectIDAccessors.h"
#include "factories/reco/ChargedLambdaReconstruction_factory.h"
#include "factories/reco/ChargedReconstructedParticleSelector_factory.h"
#include "factories/reco/ClustersToParticles_factory.h"
#include "factories/reco/FarForwardNeutralsReconstruction_factory.h"
#include "factories/reco/HadronicFinalState_factory.h"
#include "factories/reco/InclusiveKinematicsML_factory.h"
#include "factories/reco/InclusiveKinematicsReconstructed_factory.h"
#include "factories/reco/InclusiveKinematicsTruthBare_factory.h"
#include "factories/reco/InclusiveKinematicsTruthBorn_factory.h"
#include "factories/reco/InclusiveKinematicsTruthDressed_factory.h"
#include "factories/reco/JetReconstruction_factory.h"
#include "factories/reco/LGADHitCalibration_factory.h"
#include "factories/reco/LambdaReconstruction_factory.h"
#include "factories/reco/MC2ReconstructedParticle_factory.h"
#include "factories/reco/MatchClusters_factory.h"
#include "factories/reco/PrimaryVertices_factory.h"
#include "factories/reco/ReconstructedElectrons_factory.h"
#include "factories/reco/ReconstructedParticleSortKeys.h"
#include "factories/reco/ScatteredElectronsEMinusPz_factory.h"
#include "factories/reco/ScatteredElectronsTruth_factory.h"
#include "factories/reco/SecondaryVerticesHelix_factory.h"
#include "factories/reco/TrackClusterMatch_factory.h"
#include "factories/reco/TransformBreitFrame_factory.h"
#include "factories/reco/TransformLocalFFFrameToLabFrame_factory.h"
#include "factories/reco/Truthiness_factory.h"
#include "factories/reco/UndoAfterBurnerMCParticles_factory.h"
#include <cstdint>
#include <edm4eic/Cluster.h>
#include <edm4eic/InclusiveKinematics.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackClusterMatch.h>
#include <edm4eic/TrackSegment.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHit.h>
#include <podio/detail/Link.h>
#include <string>

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_reco, m) {
  m.doc() = "EICrecon reconstruction factory types";
  py::module_::import("eicrecon._core");
  bind_factory<ChargedLambdaReconstruction_factory>(m, "ChargedLambdaReconstruction");
  bind_factory<ChargedReconstructedParticleSelector_factory>(
      m, "ChargedReconstructedParticleSelector");
  bind_factory<ClustersToParticles_factory>(m, "ClustersToParticles");
  bind_factory<FarForwardNeutralsReconstruction_factory>(m, "FarForwardNeutralsReconstruction");
  bind_factory<HadronicFinalState_factory<HadronicFinalState>>(
      m, "HadronicFinalState_HadronicFinalState");
  bind_factory<InclusiveKinematicsML_factory>(m, "InclusiveKinematicsML");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsDA>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsDA");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsESigma>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsESigma");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsElectron>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsElectron");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsJB>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsJB");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsSigma>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsSigma");
  bind_factory<InclusiveKinematicsTruthBare_factory>(m, "InclusiveKinematicsTruthBare");
  bind_factory<InclusiveKinematicsTruthBorn_factory>(m, "InclusiveKinematicsTruthBorn");
  bind_factory<InclusiveKinematicsTruthDressed_factory>(m, "InclusiveKinematicsTruthDressed");
  bind_factory<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      m, "JetReconstruction_ReconstructedParticle");
  bind_factory<LGADHitCalibration_factory>(m, "LGADHitCalibration");
  bind_factory<LambdaReconstruction_factory>(m, "LambdaReconstruction");
  bind_factory<MC2ReconstructedParticle_factory>(m, "MC2ReconstructedParticle");
  bind_factory<MatchClusters_factory>(m, "MatchClusters");
  bind_factory<PrimaryVertices_factory>(m, "PrimaryVertices");
  bind_factory<ReconstructedElectrons_factory>(m, "ReconstructedElectrons");
  bind_factory<ScatteredElectronsEMinusPz_factory>(m, "ScatteredElectronsEMinusPz");
  bind_factory<ScatteredElectronsTruth_factory>(m, "ScatteredElectronsTruth");
  bind_factory<SecondaryVerticesHelix_factory>(m, "SecondaryVerticesHelix");
  bind_factory<TrackClusterMatch_factory>(m, "TrackClusterMatch");
  bind_factory<TransformBreitFrame_factory>(m, "TransformBreitFrame");
  bind_factory<TransformLocalFFFrameToLabFrame_factory>(m, "TransformLocalFFFrameToLabFrame");
  bind_factory<Truthiness_factory>(m, "Truthiness");
  bind_factory<UndoAfterBurnerMCParticles_factory>(m, "UndoAfterBurnerMCParticles");
}
