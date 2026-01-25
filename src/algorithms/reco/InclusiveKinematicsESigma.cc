// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck, Barak Schmookler

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
#include "InclusiveKinematicsESigma.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsESigma::init() {}

void InclusiveKinematicsESigma::process(const InclusiveKinematicsESigma::Input& input,
                                        const InclusiveKinematicsESigma::Output& output) const {

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
  const auto y_e  = 1. - sigma_e / (2. * ei.energy());
  const auto Q2_e = (pt_e * pt_e) / (1. - y_e);

  const auto y_sig  = sigma_h / sigma_tot;
  const auto Q2_sig = (pt_e * pt_e) / (1. - y_sig);
  const auto x_sig  = Q2_sig / (4. * ei.energy() * pi.energy() * y_sig);

  static const auto m_proton = m_particleSvc.particle(2212).mass;
  const auto Q2_esig         = Q2_e;
  const auto x_esig          = x_sig;
  const auto y_esig          = Q2_esig / (4. * ei.energy() * pi.energy() *
                                 x_esig); //equivalent to (2*ei.energy() / sigma_tot)*y_sig
  const auto nu_esig         = Q2_esig / (2. * m_proton * x_esig);
  const auto W_esig          = sqrt(m_proton * m_proton + 2 * m_proton * nu_esig - Q2_esig);
  auto kin                   = kinematics->create(x_esig, Q2_esig, W_esig, y_esig, nu_esig);
  kin.setScat(kf);

  // Debugging output
  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
