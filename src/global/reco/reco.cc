// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_reco: the factory types (and C++ helpers) used
// by the Python wiring of the reco plugin, src/python/eicrecon/plugins/reco.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4eic/Cluster.h>
#include <edm4eic/InclusiveKinematics.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4hep/MCParticle.h>
#include <podio/detail/Link.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "algorithms/reco/HadronicFinalState.h"
#include "algorithms/reco/InclusiveKinematicsDA.h"
#include "algorithms/reco/InclusiveKinematicsESigma.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "algorithms/reco/InclusiveKinematicsJB.h"
#include "algorithms/reco/InclusiveKinematicsSigma.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/FilterMatching_factory.h"
#include "factories/meta/ObjectIDAccessors.h"
#include "factories/meta/SortSubsetCollection_factory.h"
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
#include "factories/reco/Truthiness_factory.h"
#include "factories/reco/UndoAfterBurnerMCParticles_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_reco, m) {
  m.doc() = "Factory types wired by the reco plugin";
  py::module_::import("eicrecon._core");
  bind_factory<FilterMatching_factory<edm4eic::MCRecoParticleLink, eicrecon::object_id::to,
                                      edm4hep::MCParticle, eicrecon::object_id::self>>(
      m, "FilterMatching_MCRecoParticleLink_to_MCParticle_self");
  bind_factory<FilterMatching_factory<edm4eic::MCRecoParticleAssociation, eicrecon::object_id::sim,
                                      edm4hep::MCParticle, eicrecon::object_id::self>>(
      m, "FilterMatching_MCRecoParticleAssociation_sim_MCParticle_self");
  bind_factory<MC2ReconstructedParticle_factory>(m, "MC2ReconstructedParticle");
  bind_factory<CollectionCollector_factory<edm4eic::Cluster, true>>(
      m, "CollectionCollector_Cluster_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoClusterParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoClusterParticleAssociation_true");
  bind_factory<MatchClusters_factory>(m, "MatchClusters");
  bind_factory<InclusiveKinematicsTruthBare_factory>(m, "InclusiveKinematicsTruthBare");
  bind_factory<CollectionCollector_factory<edm4eic::InclusiveKinematics>>(
      m, "CollectionCollector_InclusiveKinematics");
  bind_factory<InclusiveKinematicsTruthBorn_factory>(m, "InclusiveKinematicsTruthBorn");
  bind_factory<InclusiveKinematicsTruthDressed_factory>(m, "InclusiveKinematicsTruthDressed");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsElectron>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsElectron");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsJB>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsJB");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsDA>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsDA");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsESigma>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsESigma");
  bind_factory<InclusiveKinematicsReconstructed_factory<InclusiveKinematicsSigma>>(
      m, "InclusiveKinematicsReconstructed_InclusiveKinematicsSigma");
  bind_factory<InclusiveKinematicsML_factory>(m, "InclusiveKinematicsML");
  bind_factory<ReconstructedElectrons_factory>(m, "ReconstructedElectrons");
  bind_factory<JetReconstruction_factory<edm4eic::ReconstructedParticle>>(
      m, "JetReconstruction_ReconstructedParticle");
  bind_factory<ChargedReconstructedParticleSelector_factory>(
      m, "ChargedReconstructedParticleSelector");
  bind_factory<ClustersToParticles_factory>(m, "ClustersToParticles");
  bind_factory<ScatteredElectronsTruth_factory>(m, "ScatteredElectronsTruth");
  bind_factory<ScatteredElectronsEMinusPz_factory>(m, "ScatteredElectronsEMinusPz");
  bind_factory<SortSubsetCollection_factory<edm4eic::ReconstructedParticle, reco_particle_pt>>(
      m, "SortSubsetCollection_ReconstructedParticle_reco_particle_pt");
  bind_factory<TrackClusterMatch_factory>(m, "TrackClusterMatch");
  bind_factory<TransformBreitFrame_factory>(m, "TransformBreitFrame");
  bind_factory<FarForwardNeutralsReconstruction_factory>(m, "FarForwardNeutralsReconstruction");
  bind_factory<LambdaReconstruction_factory>(m, "LambdaReconstruction");
  bind_factory<ChargedLambdaReconstruction_factory>(m, "ChargedLambdaReconstruction");
  bind_factory<HadronicFinalState_factory<HadronicFinalState>>(
      m, "HadronicFinalState_HadronicFinalState");
  bind_factory<UndoAfterBurnerMCParticles_factory>(m, "UndoAfterBurnerMCParticles");
  bind_factory<PrimaryVertices_factory>(m, "PrimaryVertices");
  bind_factory<SecondaryVerticesHelix_factory>(m, "SecondaryVerticesHelix");
  bind_factory<Truthiness_factory>(m, "Truthiness");
}
