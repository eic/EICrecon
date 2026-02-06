// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#include "UndoAfterBurner.h"

#include <Math/GenVector/Boost.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/RotationX.h>
#include <Math/GenVector/RotationY.h>
#include <Math/Vector4Dfwd.h>
#include <TMath.h>
#include <edm4hep/Vector3d.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <utility>
#include <vector>

#include "algorithms/reco/Beam.h"
#include "algorithms/reco/UndoAfterBurnerConfig.h"

void eicrecon::UndoAfterBurner::init() {}

void eicrecon::UndoAfterBurner::process(const UndoAfterBurner::Input& input,
                                        const UndoAfterBurner::Output& output) const {

  const auto [mcparts]   = input;
  auto [outputParticles] = output;

  bool pidAssumePionMass = m_cfg.m_pid_assume_pion_mass;
  double crossingAngle   = m_cfg.m_crossing_angle;
  bool correctBeamFX     = m_cfg.m_correct_beam_FX;
  bool pidUseMCTruth     = m_cfg.m_pid_use_MC_truth;

  bool hasBeamHadron = true;
  bool hasBeamLepton = true;

  //read MCParticles information and "postburn" to remove the afterburner effects.
  //The output is then the original MC input produced by the generator.

  ROOT::Math::PxPyPzEVector e_beam(0., 0., 0., 0.);
  ROOT::Math::PxPyPzEVector h_beam(0., 0., 0., 0.);

  auto incoming_lepton = find_first_beam_electron(mcparts);
  if (incoming_lepton.empty()) {
    debug("No beam electron found -- particleGun input");
    hasBeamLepton = false;
  }

  auto incoming_hadron = find_first_beam_hadron(mcparts);
  if (incoming_hadron.empty()) {
    debug("No beam hadron found -- particleGun input");
    hasBeamHadron = false;
  }

  if ((hasBeamHadron && !hasBeamLepton) || (!hasBeamHadron && hasBeamLepton)) {
    debug("Only one beam defined! Not a possible configuration!");
    return;
  }

  // Handling for FF particle gun input!!
  if (!hasBeamHadron || !hasBeamLepton) {
    for (const auto& p : *mcparts) {
      if ((p.getPDG() == 2212 || p.getPDG() == 2112)) { //look for "gun" proton/neutron
        hasBeamHadron = true;
        h_beam.SetPxPyPzE(crossingAngle * p.getEnergy(), 0.0, p.getEnergy(), p.getEnergy());
        if (p.getEnergy() > 270.0 && p.getEnergy() < 280.0) {
          hasBeamLepton = true;
          e_beam.SetPxPyPzE(0.0, 0.0, -18.0, 18.0);
        }
      }
    }

  } else {

    if (correctBeamFX) {

      h_beam.SetPxPyPzE(incoming_hadron[0].getMomentum().x, incoming_hadron[0].getMomentum().y,
                        incoming_hadron[0].getMomentum().z, incoming_hadron[0].getEnergy());
      e_beam.SetPxPyPzE(incoming_lepton[0].getMomentum().x, incoming_lepton[0].getMomentum().y,
                        incoming_lepton[0].getMomentum().z, incoming_lepton[0].getEnergy());

    } else {

      h_beam.SetPxPyPzE(crossingAngle * incoming_hadron[0].getEnergy(), 0.0,
                        incoming_hadron[0].getEnergy(), incoming_hadron[0].getEnergy());
      e_beam.SetPxPyPzE(0.0, 0.0, -incoming_lepton[0].getEnergy(), incoming_lepton[0].getEnergy());
    }
  }

  // Bail out if still no beam particles, since this leads to division by zero
  if (!hasBeamHadron && !hasBeamLepton) {
    return;
  }

  // Calculate boost vectors and rotations here
  ROOT::Math::PxPyPzEVector cm_frame_boost = e_beam + h_beam;
  ROOT::Math::Cartesian3D beta(-cm_frame_boost.Px() / cm_frame_boost.E(),
                               -cm_frame_boost.Py() / cm_frame_boost.E(),
                               -cm_frame_boost.Pz() / cm_frame_boost.E());
  ROOT::Math::Boost boostVector(beta);

  // Boost to CM frame
  e_beam = boostVector(e_beam);
  h_beam = boostVector(h_beam);

  double rotationAngleY = -1.0 * TMath::ATan2(h_beam.Px(), h_beam.Pz());
  double rotationAngleX = 1.0 * TMath::ATan2(h_beam.Py(), h_beam.Pz());

  ROOT::Math::RotationY rotationAboutY(rotationAngleY);
  ROOT::Math::RotationX rotationAboutX(rotationAngleX);

  // Boost back to proper head-on frame
  ROOT::Math::PxPyPzEVector head_on_frame_boost(0., 0., cm_frame_boost.Pz(), cm_frame_boost.E());
  ROOT::Math::Boost headOnBoostVector(head_on_frame_boost.Px() / head_on_frame_boost.E(),
                                      head_on_frame_boost.Py() / head_on_frame_boost.E(),
                                      head_on_frame_boost.Pz() / head_on_frame_boost.E());

  // Now, loop through events and apply operations to the MCparticles
  const int maxGenStatus = m_cfg.m_max_gen_status;

  // Helper lambda to check if particle should be processed
  auto shouldProcessParticle = [maxGenStatus](const edm4hep::MCParticle& p) {
    if (p.isCreatedInSimulation()) {
      return false;
    }
    // Filter by generator status to exclude background particles and conserve memory
    if (maxGenStatus >= 0 && p.getGeneratorStatus() > maxGenStatus) {
      return false;
    }
    return true;
  };

  // Custom comparator for MCParticle to ensure deterministic ordering
  auto mcParticleCompare = [](const edm4hep::MCParticle& a, const edm4hep::MCParticle& b) {
    auto id_a = a.getObjectID();
    auto id_b = b.getObjectID();
    if (id_a.collectionID != id_b.collectionID) {
      return id_a.collectionID < id_b.collectionID;
    }
    return id_a.index < id_b.index;
  };

  // Map from input MCParticle to output MCParticle index
  std::map<edm4hep::MCParticle, size_t, decltype(mcParticleCompare)> inputToOutputMap(
      mcParticleCompare);

  // First pass: create all output particles
  for (const auto& p : *mcparts) {
    if (!shouldProcessParticle(p)) {
      continue;
    }

    ROOT::Math::PxPyPzEVector mc(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z,
                                 p.getEnergy());

    mc = boostVector(mc);
    mc = rotationAboutY(mc);
    mc = rotationAboutX(mc);
    mc = headOnBoostVector(mc);

    decltype(edm4hep::MCParticleData::momentum) mcMom(mc.Px(), mc.Py(), mc.Pz());
    edm4hep::MutableMCParticle MCTrack(p.clone());
    MCTrack.setMomentum(mcMom);

    if (pidUseMCTruth) {
      MCTrack.setPDG(p.getPDG());
      MCTrack.setMass(p.getMass());
    }
    if (!pidUseMCTruth && pidAssumePionMass) {
      MCTrack.setPDG(211);
      MCTrack.setMass(0.13957);
    }

    outputParticles->push_back(MCTrack);
    // Store mapping from input particle to output particle index
    inputToOutputMap[p] = outputParticles->size() - 1;
  }

  // Second pass: establish parent-daughter relationships
  for (const auto& p : *mcparts) {
    if (!shouldProcessParticle(p)) {
      continue;
    }

    // Get the output particle corresponding to this input particle
    auto outputIter = inputToOutputMap.find(p);
    if (outputIter == inputToOutputMap.end()) {
      continue;
    }
    auto outputParticle = outputParticles->at(outputIter->second);

    // Add parent relationships
    for (const auto& parent : p.getParents()) {
      auto parentIter = inputToOutputMap.find(parent);
      if (parentIter != inputToOutputMap.end()) {
        auto outputParent = outputParticles->at(parentIter->second);
        outputParticle.addToParents(outputParent);
      }
    }

    // Add daughter relationships
    for (const auto& daughter : p.getDaughters()) {
      auto daughterIter = inputToOutputMap.find(daughter);
      if (daughterIter != inputToOutputMap.end()) {
        auto outputDaughter = outputParticles->at(daughterIter->second);
        outputParticle.addToDaughters(outputDaughter);
      }
    }
  }
}
