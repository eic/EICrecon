// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>
#include <gsl/pointers>

#include "Beam.h"
#include "InclusiveKinematicsTruth.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsTruth::init() {}

void InclusiveKinematicsTruth::process(const InclusiveKinematicsTruth::Input& input,
                                       const InclusiveKinematicsTruth::Output& output) const {

  const auto [mcparts] = input;
  auto [kinematics]    = output;

  // Loop over generated particles to get incoming electron and proton beams
  // and the scattered electron. In the presence of QED radition on the incoming
  // or outgoing electron line, the vertex kinematics will be different than the
  // kinematics calculated using the scattered electron as done here.
  // Also need to update for CC events.

  // Get incoming electron beam
  const auto ei_coll = find_first_beam_electron(mcparts);
  if (ei_coll.empty()) {
    debug("No beam electron found");
    return;
  }
  const auto ei_p           = ei_coll[0].getMomentum();
  const auto ei_p_mag       = edm4hep::utils::magnitude(ei_p);
  static const auto ei_mass = m_particleSvc.particle(11).mass;
  const PxPyPzEVector ei(ei_p.x, ei_p.y, ei_p.z, std::hypot(ei_p_mag, ei_mass));

  // Get incoming hadron beam
  const auto pi_coll = find_first_beam_hadron(mcparts);
  if (pi_coll.empty()) {
    debug("No beam hadron found");
    return;
  }
  const auto pi_p     = pi_coll[0].getMomentum();
  const auto pi_p_mag = edm4hep::utils::magnitude(pi_p);
  const auto pi_mass  = m_particleSvc.particle(pi_coll[0].getPDG()).mass;
  const PxPyPzEVector pi(pi_p.x, pi_p.y, pi_p.z, std::hypot(pi_p_mag, pi_mass));

  // Get first scattered electron
  // Scattered electron. Currently taken as first status==1 electron in HEPMC record,
  // which seems to be correct based on a cursory glance at the Pythia8 output. In the future,
  // it may be better to trace back each final-state electron and see which one originates from
  // the beam.
  const auto ef_coll = find_first_scattered_electron(mcparts);
  if (ef_coll.empty()) {
    debug("No truth scattered electron found");
    return;
  }
  const auto ef_p           = ef_coll[0].getMomentum();
  const auto ef_p_mag       = edm4hep::utils::magnitude(ef_p);
  static const auto ef_mass = m_particleSvc.particle(11).mass;
  const PxPyPzEVector ef(ef_p.x, ef_p.y, ef_p.z, std::hypot(ef_p_mag, ef_mass));

  // DIS kinematics calculations
  const auto q        = ei - ef;
  const auto q_dot_pi = q.Dot(pi);
  const auto Q2       = -q.Dot(q);
  const auto y        = q_dot_pi / ei.Dot(pi);
  const auto nu       = q_dot_pi / pi_mass;
  const auto x        = Q2 / (2. * q_dot_pi);
  const auto W        = sqrt(pi_mass * pi_mass + 2. * q_dot_pi - Q2);
  auto kin            = kinematics->create(x, Q2, W, y, nu);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
