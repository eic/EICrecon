// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

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
#include "Boost.h"
#include "InclusiveKinematicsDA.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsDA::init() {}

void InclusiveKinematicsDA::process(const InclusiveKinematicsDA::Input& input,
                                    const InclusiveKinematicsDA::Output& output) const {

  const auto [mc_beam_electrons, mc_beam_protons, escat, hfs] = input;
  auto [out_kinematics]                                       = output;

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

  // Get boost to colinear frame
  auto boost = determine_boost(ei, pi);

  // Get electron angle
  if (escat->empty()) {
    debug("No scattered electron found");
    return;
  }
  auto kf = escat->at(0);
  PxPyPzEVector e_lab(kf.getMomentum().x, kf.getMomentum().y, kf.getMomentum().z, kf.getEnergy());
  PxPyPzEVector e_boosted = apply_boost(boost, e_lab);
  auto theta_e            = e_boosted.Theta();

  // Get hadronic final state variables
  if (hfs->empty()) {
    debug("No hadronic final state found");
    return;
  }
  auto sigma_h = hfs->at(0).getSigma();
  auto gamma_h = hfs->at(0).getGamma();

  // Sigma zero or negative
  if (sigma_h <= 0) {
    debug("Sigma zero or negative");
    return;
  }

  // Calculate kinematic variables
  static const auto m_proton = m_particleSvc.particle(2212).mass;
  const auto y_da            = tan(gamma_h / 2.) / (tan(theta_e / 2.) + tan(gamma_h / 2.));
  const auto Q2_da           = 4. * ei.energy() * ei.energy() * (1. / tan(theta_e / 2.)) *
                     (1. / (tan(theta_e / 2.) + tan(gamma_h / 2.)));
  const auto x_da  = Q2_da / (4. * ei.energy() * pi.energy() * y_da);
  const auto nu_da = Q2_da / (2. * m_proton * x_da);
  const auto W_da  = sqrt(m_proton * m_proton + 2 * m_proton * nu_da - Q2_da);
  auto kin         = out_kinematics->create(x_da, Q2_da, W_da, y_da, nu_da);
  kin.setScat(kf);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
