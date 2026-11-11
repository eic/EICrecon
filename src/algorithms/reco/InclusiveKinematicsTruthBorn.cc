// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/MCParticleCollection.h>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <tuple>
#include <vector>

#include "InclusiveKinematicsTruthBorn.h"
#include "TruthLeptonLine.h"

using ROOT::Math::PxPyPzEVector;

namespace {
// Maximum momentum difference between the momentum transfer and the exchanged boson
constexpr double boson_momentum_tolerance = 10 * edm4eic::unit::MeV;
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

  // Four-momentum transfer at the hadronic vertex: between the hard-scattering leptons when the
  // record marks them, otherwise from momentum conservation along the lepton line
  PxPyPzEVector q;
  if (truth->hard_in.has_value() && truth->hard_out.has_value() &&
      *truth->hard_in < *truth->hard_out) {
    const auto& l_in  = truth->line[*truth->hard_in];
    const auto& l_out = truth->line[*truth->hard_out];
    q = four_momentum(l_in, lepton_mass(l_in)) - four_momentum(l_out, lepton_mass(l_out));
    trace("q from hard-scattering leptons");
  } else {
    PxPyPzEVector photons;
    for (const auto& photon : truth->photons) {
      photons += four_momentum(photon.particle);
    }
    q = ei - ef - photons;
    trace("q from lepton line with {} radiated photons", truth->photons.size());
  }

  // Check against the exchanged boson, when stored. Only its momentum is compared: its energy
  // follows from its stored mass, which generators write with limited precision.
  if (truth->boson.has_value()) {
    const auto& p_boson = truth->boson->getMomentum();
    const double dp     = std::hypot(p_boson.x - q.Px(), p_boson.y - q.Py(), p_boson.z - q.Pz());
    if (dp > boson_momentum_tolerance) {
      warning("Momentum transfer differs from exchanged boson (PDG {}) by {} GeV",
              truth->boson->getPDG(), dp);
    }
  }

  // DIS kinematics calculations
  const auto dis = dis_variables(ei, pi_beam.momentum, pi_beam.mass, q);
  auto kin       = kinematics->create(dis.x, dis.Q2, dis.W, dis.y, dis.nu);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
