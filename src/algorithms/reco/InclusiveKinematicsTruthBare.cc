// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <fmt/core.h>
#include <cstdlib>
#include <gsl/pointers>
#include <optional>

#include "InclusiveKinematicsTruthBare.h"
#include "TruthLeptonLine.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsTruthBare::init() {}

void InclusiveKinematicsTruthBare::process(
    const InclusiveKinematicsTruthBare::Input& input,
    const InclusiveKinematicsTruthBare::Output& output) const {

  const auto [mcparts] = input;
  auto [kinematics]    = output;

  // Kinematics from the beam lepton and the bare final-state scattered lepton of the signal
  // interaction, q = k - k'. In the presence of QED radiation on the incoming or outgoing
  // lepton line, the vertex kinematics will be different than the kinematics calculated here;
  // see InclusiveKinematicsTruthBorn and InclusiveKinematicsTruthDressed.

  // Get lepton line of the signal interaction
  const auto truth = find_lepton_line_truth(*mcparts);
  if (!truth.has_value()) {
    debug("No beam electron with a final-state scattered lepton found");
    return;
  }

  // Get incoming electron beam
  static const auto ei_mass = m_particleSvc.particle(11).mass;
  const PxPyPzEVector ei    = four_momentum(truth->beam_lepton(), ei_mass);

  // Get incoming hadron beam
  if (!truth->beam_hadron.has_value()) {
    debug("No beam hadron found");
    return;
  }
  const auto& pi_particle = *truth->beam_hadron;
  const auto pi_beam      = beam_per_nucleon(
      pi_particle,
      is_ion(pi_particle.getPDG()) ? 0. : m_particleSvc.particle(pi_particle.getPDG()).mass);
  const PxPyPzEVector pi = pi_beam.momentum;

  // Get scattered lepton: the final-state end of the lepton line from the beam electron
  // (the electron for neutral-current, the neutrino for charged-current events)
  const auto& ef_particle = truth->scattered_lepton();
  const auto ef_mass      = m_particleSvc.particle(std::abs(ef_particle.getPDG())).mass;
  const PxPyPzEVector ef  = four_momentum(ef_particle, ef_mass);

  // DIS kinematics calculations
  const auto q   = ei - ef;
  const auto dis = dis_variables(ei, pi, pi_beam.mass, q);
  auto kin       = kinematics->create(dis.x, dis.Q2, dis.W, dis.y, dis.nu);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
