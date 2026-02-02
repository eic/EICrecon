// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "Beam.h"
#include "InclusiveKinematicsJB.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsJB::init() {}

void InclusiveKinematicsJB::process(const InclusiveKinematicsJB::Input& input,
                                    const InclusiveKinematicsJB::Output& output) const {

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

  // Get hadronic final state variables
  if (hfs->empty()) {
    debug("No hadronic final state found");
    return;
  }
  auto sigma_h = hfs->at(0).getSigma();
  auto ptsum   = hfs->at(0).getPT();

  // Sigma zero or negative
  if (sigma_h <= 0) {
    debug("Sigma zero or negative");
    return;
  }

  // Calculate kinematic variables
  static const auto m_proton = m_particleSvc.particle(2212).mass;
  const auto y_jb            = sigma_h / (2. * ei.energy());
  if (y_jb >= 1) {
    // y > 0 is mathematically guaranteed by sigma_h > 0, but y < 1 is not
    debug("InclusiveKinematicsJB: event with y > 1 skipped");
    return;
  }
  const auto Q2_jb = ptsum * ptsum / (1. - y_jb);
  const auto x_jb  = Q2_jb / (4. * ei.energy() * pi.energy() * y_jb);
  if (x_jb >= 1) {
    // x > 0 is mathematically guaranteed by 0 < y < 1, but x < 1 is not
    debug("InclusiveKinematicsJB: event with x > 1 skipped");
    return;
  }
  const auto nu_jb = Q2_jb / (2. * m_proton * x_jb);
  const auto W_jb  = sqrt(m_proton * m_proton + 2 * m_proton * nu_jb - Q2_jb);
  auto kin         = out_kinematics->create(x_jb, Q2_jb, W_jb, y_jb, nu_jb);
  if (escat->empty()) {
    debug("No scattered electron found");
  } else {
    kin.setScat(escat->at(0));
  }

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
