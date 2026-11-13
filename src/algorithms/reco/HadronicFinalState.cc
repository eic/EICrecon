// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <algorithm>
#include <optional>
#include <tuple>

#include "Beam.h"
#include "Boost.h"
#include "HadronicFinalState.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void HadronicFinalState::init() {}

void HadronicFinalState::process(const HadronicFinalState::Input& input,
                                 const HadronicFinalState::Output& output) const {

  const auto [mc_beam_electrons, mc_beam_hadrons, escat, rcparts] = input;
  auto [hadronicfinalstate]                                       = output;

  // Get first (should be only) beam electron
  if (mc_beam_electrons->empty()) {
    debug("No beam electron found");
    return;
  }
  const auto& ei_particle = (*mc_beam_electrons)[0];
  const PxPyPzEVector ei(round_beam_four_momentum(ei_particle.getMomentum(),
                                                  m_particleSvc.particle(ei_particle.getPDG()).mass,
                                                  electron_beam_pz_set, 0.0));

  // Get first (should be only) beam hadron
  if (mc_beam_hadrons->empty()) {
    debug("No beam hadron found");
    return;
  }
  const auto& pi_particle = (*mc_beam_hadrons)[0];
  const PxPyPzEVector pi(round_beam_four_momentum(pi_particle.getMomentum(),
                                                  m_particleSvc.particle(pi_particle.getPDG()).mass,
                                                  hadron_beam_pz_set, m_crossingAngle));

  // Scattered electron, excluded from the hadronic final state, if any
  std::optional<edm4eic::ReconstructedParticle> ef;
  if (escat->empty()) {
    debug("No scattered electron, all particles are included");
  } else {
    ef = (*escat)[0];
  }
  const auto is_scattered_electron = [&ef](const edm4eic::ReconstructedParticle& p) {
    if (!ef.has_value()) {
      return false;
    }
    if (p.getObjectID() == ef->getObjectID()) {
      return true;
    }
    return std::ranges::any_of(p.getTracks(), [&ef](const auto& track) {
      return std::ranges::any_of(ef->getTracks(), [&track](const auto& ef_track) {
        return ef_track.getObjectID() == track.getObjectID();
      });
    });
  };

  // Get boost to colinear frame
  auto boost = determine_boost(ei, pi);

  auto hfs = hadronicfinalstate->create(0., 0., 0.);

  // Sum in colinear frame
  PxPyPzEVector sum;
  for (const auto& p : *rcparts) {
    if (!is_scattered_electron(p)) {
      // Lorentz vector in lab frame
      PxPyPzEVector hf_lab(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, p.getEnergy());
      // Boost to colinear frame
      sum += apply_boost(boost, hf_lab);

      hfs.addToHadrons(p);
    }
  }

  // Hadronic final state calculations
  const auto [sigma, pT, gamma] = hadronic_final_state_sums(sum);
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
