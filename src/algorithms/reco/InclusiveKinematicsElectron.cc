// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wouter Deconinck, Tooba Ali

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "Beam.h"
#include "InclusiveKinematicsElectron.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsElectron::init() {}

void InclusiveKinematicsElectron::process(const InclusiveKinematicsElectron::Input& input,
                                          const InclusiveKinematicsElectron::Output& output) const {

  const auto [mc_beam_electrons, mc_beam_protons, escat, hfs] = input;
  auto [kinematics]                                           = output;

  // 1. find_if
  //const auto mc_first_electron = std::find_if(
  //  mcparts.begin(),
  //  mcparts.end(),
  //  [](const auto& p){ return p.getPDG() == 11; });

  // 2a. simple loop over iterator (post-increment)
  //auto mc_first_electron = mcparts.end();
  //for (auto p = mcparts.begin(); p != mcparts.end(); p++) {
  //  if (p.getPDG() == 11) {
  //    mc_first_electron = p;
  //    break;
  //  }
  //}
  // 2b. simple loop over iterator (pre-increment)
  //auto mc_first_electron = mcparts.end();
  //for (auto p = mcparts.begin(); p != mcparts.end(); ++p) {
  //  if (p.getPDG() == 11) {
  //    mc_first_electron = p;
  //    break;
  //  }
  //}

  // 3. pre-initialized simple loop
  //auto mc_first_electron = mcparts.begin();
  //for (; mc_first_electron != mcparts.end(); ++mc_first_electron) {
  //  if (mc_first_electron.getPDG() == 11) {
  //    break;
  //  }
  //}

  // 4a. iterator equality
  //if (mc_first_electron == mcparts.end()) {
  //  debug() << "No electron found" << endmsg;
  //  return StatusCode::FAILURE;
  //}
  // 4b. iterator inequality
  //if (!(mc_first_electron != mcparts.end())) {
  //  debug() << "No electron found" << endmsg;
  //  return StatusCode::FAILURE;
  //}

  // 5. ranges and views
  //auto is_electron = [](const auto& p){ return p.getPDG() == 11; };
  //for (const auto& e: mcparts | std::views::filter(is_electron)) {
  //  break;
  //}

  // Get first (should be only) beam electron
  if (mc_beam_electrons->empty()) {
    debug("No beam electron found");
    return;
  }
  const auto& ei_particle = (*mc_beam_electrons)[0];
  const PxPyPzEVector ei(round_beam_four_momentum(ei_particle.getMomentum(),
                                                  m_particleSvc.particle(ei_particle.getPDG()).mass,
                                                  {-5.0, -10.0, -18.0}, 0.0));

  // Get first (should be only) beam proton
  if (mc_beam_protons->empty()) {
    debug("No beam hadron found");
    return;
  }
  const auto& pi_particle = (*mc_beam_protons)[0];
  const PxPyPzEVector pi(round_beam_four_momentum(pi_particle.getMomentum(),
                                                  m_particleSvc.particle(pi_particle.getPDG()).mass,
                                                  {41.0, 100.0, 275.0}, m_crossingAngle));

  // Get scattered electron
  std::vector<PxPyPzEVector> electrons;
  for (const auto& p : *escat) {
    electrons.emplace_back(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
    break;
  }

  // If no scattered electron was found
  if (electrons.empty()) {
    debug("No scattered electron found");
    return;
  }

  // DIS kinematics calculations
  static const auto m_proton = m_particleSvc.particle(2212).mass;
  const auto ef              = electrons.front();
  const auto q               = ei - ef;
  const auto q_dot_pi        = q.Dot(pi);
  const auto Q2              = -q.Dot(q);
  const auto y               = q_dot_pi / ei.Dot(pi);
  const auto nu              = q_dot_pi / m_proton;
  const auto x               = Q2 / (2. * q_dot_pi);
  const auto W               = sqrt(m_proton * m_proton + 2. * q_dot_pi - Q2);
  auto kin                   = kinematics->create(x, Q2, W, y, nu);
  kin.setScat(escat->at(0));

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
