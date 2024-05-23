// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_VERSION_MAJOR >= 6

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>

#include "Beam.h"
#include "Boost.h"
#include "InclusiveKinematicsJB.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

  void InclusiveKinematicsJB::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    // m_pidSvc = service("ParticleSvc");
    // if (!m_pidSvc) {
    //   m_log->debug("Unable to locate Particle Service. "
    //     "Make sure you have ParticleSvc in the configuration.");
    // }
  }

  void InclusiveKinematicsJB::process(
      const InclusiveKinematicsJB::Input& input,
      const InclusiveKinematicsJB::Output& output) const {

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

    // Get hadronic final state variables
    auto sigma_h = hfs->at(0).getSigma();
    auto ptsum = hfs->at(0).getPT();
    auto gamma_h = hfs->at(0).getGamma();

    // Sigma zero or negative
    if (sigma_h <= 0) {
      m_log->debug("Sigma zero or negative");
      return;
    }

    // Calculate kinematic variables
    const auto y_jb = sigma_h / (2.*ei.energy());
    const auto Q2_jb = ptsum*ptsum / (1. - y_jb);
    const auto x_jb = Q2_jb / (4.*ei.energy()*pi.energy()*y_jb);
    const auto nu_jb = Q2_jb / (2.*m_proton*x_jb);
    const auto W_jb = sqrt(m_proton*m_proton + 2*m_proton*nu_jb - Q2_jb);
    auto kin = kinematics->create(x_jb, Q2_jb, W_jb, y_jb, nu_jb);
    kin.setScat(escat->at(0));

    m_log->debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(),
            kin.getQ2(), kin.getW(), kin.getY(), kin.getNu());
  }

}
#endif
