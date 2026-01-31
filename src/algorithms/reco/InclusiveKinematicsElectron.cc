// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wouter Deconinck, Tooba Ali

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
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

  const auto [mcparts, escat, hfs] = input;
  auto [kinematics]                = output;

  // Get incoming electron beam
  const auto ei_coll = find_first_beam_electron(mcparts);
  if (ei_coll.empty()) {
    debug("No beam electron found");
    return;
  }
  const PxPyPzEVector ei(round_beam_four_momentum(ei_coll[0].getMomentum(),
                                                  m_particleSvc.particle(ei_coll[0].getPDG()).mass,
                                                  {-5.0, -10.0, -18.0}, 0.0));

  // Get incoming hadron beam
  const auto pi_coll = find_first_beam_hadron(mcparts);
  if (pi_coll.empty()) {
    debug("No beam hadron found");
    return;
  }
  const PxPyPzEVector pi(round_beam_four_momentum(pi_coll[0].getMomentum(),
                                                  m_particleSvc.particle(pi_coll[0].getPDG()).mass,
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
