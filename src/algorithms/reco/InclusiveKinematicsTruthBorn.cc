// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/MCParticleCollection.h>
#include <fmt/core.h>
#include <cmath>
#include <cstdlib>
#include <gsl/pointers>
#include <optional>
#include <vector>

#include "InclusiveKinematicsTruthBorn.h"
#include "TruthLeptonLine.h"

using ROOT::Math::PxPyPzEVector;

namespace {
// Maximum momentum difference between the exchanged boson and the lepton-line balance
constexpr double boson_momentum_tolerance = 1 * edm4eic::unit::MeV;
// Relative precision of the stored mass squared of the exchanged boson (conservative)
constexpr double boson_mass_precision = 1e-5;
} // namespace

namespace eicrecon {

void InclusiveKinematicsTruthBorn::init() {}

void InclusiveKinematicsTruthBorn::process(
    const InclusiveKinematicsTruthBorn::Input& input,
    const InclusiveKinematicsTruthBorn::Output& output) const {

  const auto [mcparts] = input;
  auto [kinematics]    = output;

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

  // Get scattered lepton
  const auto lepton_mass = [this](const edm4hep::MCParticle& p) {
    return m_particleSvc.particle(std::abs(p.getPDG())).mass;
  };
  const auto& ef_particle = truth->scattered_lepton();
  const PxPyPzEVector ef  = four_momentum(ef_particle, lepton_mass(ef_particle));

  // Four-momentum transfer from momentum conservation along the lepton line
  PxPyPzEVector photons;
  for (const auto& photon : truth->photons) {
    photons += four_momentum(photon.particle);
  }
  const PxPyPzEVector q_line = ei - ef - photons;

  // Four-momentum transfer at the hadronic vertex
  PxPyPzEVector q;
  if (truth->boson.has_value()) {
    // The boson energy follows from its stored momentum and mass. Generators compute and write
    // that mass with limited precision, which determines the energy poorly when it is small
    // compared to the momentum. Use the balance along the lepton line when it agrees with the
    // boson within that precision; otherwise the balance is off (e.g. off-shell photons written
    // with zero mass, or photons missing from the lepton line) and the boson is used.
    const auto q_boson   = boson_four_momentum(*truth->boson, q_line.E());
    const double sigma_E = boson_mass_precision * (q_boson.P2() + std::abs(q_boson.M2())) /
                           (2. * std::abs(q_boson.E()));
    if ((q_boson - q_line).P() < boson_momentum_tolerance &&
        std::abs(q_boson.E() - q_line.E()) < sigma_E) {
      q = q_line;
      trace("q from lepton line, confirmed by exchanged boson (PDG {})", truth->boson->getPDG());
    } else {
      q = q_boson;
      trace("q from exchanged boson (PDG {})", truth->boson->getPDG());
    }
  } else if (truth->hard_in.has_value() && truth->hard_out.has_value() &&
             *truth->hard_in < *truth->hard_out) {
    const auto& l_in  = truth->line[*truth->hard_in];
    const auto& l_out = truth->line[*truth->hard_out];
    q = four_momentum(l_in, lepton_mass(l_in)) - four_momentum(l_out, lepton_mass(l_out));
    trace("q from hard-scattering leptons");
  } else {
    q = q_line;
    trace("q from lepton line with {} radiated photons", truth->photons.size());
  }

  // DIS kinematics calculations
  const auto dis = dis_variables(ei, pi_beam.momentum, pi_beam.mass, q);
  auto kin       = kinematics->create(dis.x, dis.Q2, dis.W, dis.y, dis.nu);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
