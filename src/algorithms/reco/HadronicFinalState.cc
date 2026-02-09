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
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "HadronicFinalState.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void HadronicFinalState::init() {}

void HadronicFinalState::process(const HadronicFinalState::Input& input,
                                 const HadronicFinalState::Output& output) const {

  const auto [mc_beam_electrons, mc_beam_protons, mcparts, rcparts, rcassoc] = input;
  auto [hadronicfinalstate]                                                  = output;

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

  // Get first scattered electron from full MCParticles collection
  if (mcparts == nullptr) {
    debug("No MCParticles collection available");
    return;
  }
  const auto ef_coll = find_first_scattered_electron(mcparts);
  if (ef_coll.empty()) {
    debug("No truth scattered electron found");
    return;
  }

  // Check if associations are available
  if (!rcassoc) {
    debug("No associations available");
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
    debug("Truth scattered electron not in reconstructed particles");
    return;
  }
  const auto ef_rc{ef_assoc->getRec()};
  const auto ef_rc_id{ef_rc.getObjectID().index};

  // Sums in colinear frame
  double pxsum = 0;
  double pysum = 0;
  double pzsum = 0;
  double Esum  = 0;

  // Get boost to colinear frame
  auto boost = determine_boost(ei, pi);

  auto hfs = hadronicfinalstate->create(0., 0., 0.);

  for (const auto& p : *rcparts) {
    // Check if it's the scattered electron
    if (p.getObjectID().index != ef_rc_id) {
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
  }

  // Hadronic final state calculations
  auto sigma = Esum - pzsum;
  auto pT    = sqrt(pxsum * pxsum + pysum * pysum);
  auto gamma = acos((pT * pT - sigma * sigma) / (pT * pT + sigma * sigma));

  hfs.setSigma(sigma);
  hfs.setPT(pT);
  hfs.setGamma(gamma);

  // Sigma zero or negative
  if (sigma <= 0) {
    debug("Sigma zero or negative");
    return;
  }

  debug("sigma_h, pT_h, gamma_h = {},{},{}", hfs.getSigma(), hfs.getPT(), hfs.getGamma());
}

} // namespace eicrecon
