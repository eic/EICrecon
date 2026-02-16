// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck, Barak Schmookler

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "InclusiveKinematicsSigma.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsSigma::init() {}

void InclusiveKinematicsSigma::process(const InclusiveKinematicsSigma::Input& input,
                                       const InclusiveKinematicsSigma::Output& output) const {

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

  // Get electron variables
  if (escat->empty()) {
    debug("No scattered electron found");
    return;
  }
  auto kf = escat->at(0);
  PxPyPzEVector e_lab(kf.getMomentum().x, kf.getMomentum().y, kf.getMomentum().z, kf.getEnergy());
  PxPyPzEVector e_boosted = apply_boost(boost, e_lab);
  auto pt_e               = e_boosted.Pt();
  auto sigma_e            = e_boosted.E() - e_boosted.Pz();

  // Get hadronic final state variables
  if (hfs->empty()) {
    debug("No hadronic final state found");
    return;
  }
  auto sigma_h = hfs->at(0).getSigma();

  if (sigma_h <= 0) {
    debug("No scattered electron found or sigma zero or negative");
    return;
  }

  auto sigma_tot = sigma_e + sigma_h;

  // Calculate kinematic variables
  static const auto m_proton = m_particleSvc.particle(2212).mass;
  const auto y_sig           = sigma_h / sigma_tot;
  const auto Q2_sig          = (pt_e * pt_e) / (1. - y_sig);
  const auto x_sig           = Q2_sig / (4. * ei.energy() * pi.energy() * y_sig);
  const auto nu_sig          = Q2_sig / (2. * m_proton * x_sig);
  const auto W_sig           = sqrt(m_proton * m_proton + 2 * m_proton * nu_sig - Q2_sig);
  auto kin                   = out_kinematics->create(x_sig, Q2_sig, W_sig, y_sig, nu_sig);
  kin.setScat(kf);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
