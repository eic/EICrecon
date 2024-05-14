// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck, Barak Schmookler

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>

#include "Beam.h"
#include "Boost.h"
#include "InclusiveKinematicsSigma.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

  void InclusiveKinematicsSigma::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    // m_pidSvc = service("ParticleSvc");
    // if (!m_pidSvc) {
    //   m_log->debug("Unable to locate Particle Service. "
    //     "Make sure you have ParticleSvc in the configuration.");
    // }
  }

  void InclusiveKinematicsSigma::process(
      const InclusiveKinematicsSigma::Input& input,
      const InclusiveKinematicsSigma::Output& output) const {

    const auto [mcparts, escat, hfs] = input;
    auto [kinematics] = output;

    // Get incoming electron beam
    const auto ei_coll = find_first_beam_electron(mcparts);
    if (ei_coll.size() == 0) {
      m_log->debug("No beam electron found");
      return;
    }
    const PxPyPzEVector ei(
      round_beam_four_momentum(
        ei_coll[0].getMomentum(),
        m_electron,
        {-5.0, -10.0, -18.0},
        0.0)
      );

    // Get incoming hadron beam
    const auto pi_coll = find_first_beam_hadron(mcparts);
    if (pi_coll.size() == 0) {
      m_log->debug("No beam hadron found");
      return;
    }
    const PxPyPzEVector pi(
      round_beam_four_momentum(
        pi_coll[0].getMomentum(),
        pi_coll[0].getPDG() == 2212 ? m_proton : m_neutron,
        {41.0, 100.0, 275.0},
        m_crossingAngle)
      );

    // Get boost to colinear frame
    auto boost = determine_boost(ei, pi);

    // Get electron variables
    auto kf = escat->at(0);
    PxPyPzEVector e_lab(kf.getMomentum().x, kf.getMomentum().y, kf.getMomentum().z, kf.getEnergy());
    PxPyPzEVector e_boosted = apply_boost(boost, e_lab);
    auto pt_e = e_boosted.Pt();
    auto sigma_e = e_boosted.E() - e_boosted.Pz();

    // Get hadronic final state variables 
    auto sigma_h = hfs->at(0).getSigma();
    auto ptsum = hfs->at(0).getPT();
    auto gamma_h = hfs->at(0).getGamma();
    
    if (sigma_h <= 0) {
      m_log->debug("No scattered electron found or sigma zero or negative");
      return;
    }

    auto sigma_tot = sigma_e + sigma_h;

    // Calculate kinematic variables
    const auto y_sig = sigma_h / sigma_tot;
    const auto Q2_sig = (pt_e*pt_e) / (1. - y_sig);
    const auto x_sig = Q2_sig / (4.*ei.energy()*pi.energy()*y_sig);
    const auto nu_sig = Q2_sig / (2.*m_proton*x_sig);
    const auto W_sig = sqrt(m_proton*m_proton + 2*m_proton*nu_sig - Q2_sig);
    auto kin = kinematics->create(x_sig, Q2_sig, W_sig, y_sig, nu_sig);
    kin.setScat(kf);

    m_log->debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(),
            kin.getQ2(), kin.getW(), kin.getY(), kin.getNu());
  }

} // namespace Jug::Reco
