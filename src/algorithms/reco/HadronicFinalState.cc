// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "HadronicFinalState.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

  void HadronicFinalState::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    // m_pidSvc = service("ParticleSvc");
    // if (!m_pidSvc) {
    //   m_log->debug("Unable to locate Particle Service. "
    //     "Make sure you have ParticleSvc in the configuration.");
    // }
  }

  void HadronicFinalState::process(
      const HadronicFinalState::Input& input,
      const HadronicFinalState::Output& output) const {

    const auto [mcparts, rcparts, rcassoc] = input;
    auto [hadronicfinalstate] = output;

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

    // Get first scattered electron
    const auto ef_coll = find_first_scattered_electron(mcparts);
    if (ef_coll.size() == 0) {
      m_log->debug("No truth scattered electron found");
      return;
    }
    // Associate first scattered electron with reconstructed electrons
    //const auto ef_assoc = std::find_if(
    //  rcassoc->begin(),
    //  rcassoc->end(),
    //  [&ef_coll](const auto& a){ return a.getSim().getObjectID() == ef_coll[0].getObjectID(); });
    auto ef_assoc = rcassoc->begin();
    for (; ef_assoc != rcassoc->end(); ++ef_assoc) {
      if (ef_assoc->getSim().getObjectID() == ef_coll[0].getObjectID()) {
        break;
      }
    }
    if (!(ef_assoc != rcassoc->end())) {
      m_log->debug("Truth scattered electron not in reconstructed particles");
      return;
    }
    const auto ef_rc{ef_assoc->getRec()};
    const auto ef_rc_id{ef_rc.getObjectID().index};

    // Sums in colinear frame
    double pxsum = 0;
    double pysum = 0;
    double pzsum = 0;
    double Esum = 0;

    // Get boost to colinear frame
    auto boost = determine_boost(ei, pi);

    auto hfs = hadronicfinalstate->create(0., 0., 0.);

    for (const auto& p: *rcparts) {

      bool isHadron = true;
      // Check if it's the scattered electron
      if (p.getObjectID().index == ef_rc_id) isHadron = false;
      // Check for non-hadron PDG codes
      if (p.getPDG() == 11) isHadron = false;
      if (p.getPDG() == 22) isHadron = false;
      if (p.getPDG() == 13) isHadron = false;
      // If it's the scattered electron or not a hadron, skip
      if(!isHadron) continue;

      // Lorentz vector in lab frame
      PxPyPzEVector hf_lab(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
      // Boost to colinear frame
      PxPyPzEVector hf_boosted = apply_boost(boost, hf_lab);

      pxsum += hf_boosted.Px();
      pysum += hf_boosted.Py();
      pzsum += hf_boosted.Pz();
      Esum += hf_boosted.E();

      hfs.addToHadrons(p);

    }

    // Hadronic final state calculations
    auto sigma = Esum - pzsum;
    auto pT = sqrt(pxsum*pxsum + pysum*pysum);
    auto gamma = (pT*pT - sigma*sigma)/(pT*pT + sigma*sigma);

    hfs.setSigma(sigma);
    hfs.setPT(pT);
    hfs.setGamma(gamma);

    // Sigma zero or negative
    if (sigma <= 0) {
      m_log->debug("Sigma zero or negative");
      return;
    }

    m_log->debug("sigma_h, pT_h, gamma_h = {},{},{}", hfs.getSigma(), hfs.getPT(), hfs.getGamma());

  }

} // namespace Jug::Reco
