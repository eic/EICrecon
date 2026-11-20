// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <podio/ObjectID.h>
#include <optional>
#include <ranges>
#include <tuple>
#include <vector>

#include "Boost.h"
#include "HadronicFinalStateSums.h"
#include "HadronicFinalStateTruth.h"
#include "TruthLeptonLine.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void HadronicFinalStateTruth::init() {}

void HadronicFinalStateTruth::process(const HadronicFinalStateTruth::Input& input,
                                      const HadronicFinalStateTruth::Output& output) const {

  const auto [mcparts]      = input;
  auto [hadronicfinalstate] = output;

  // Get lepton line of the signal interaction
  const auto truth = find_lepton_line_truth(*mcparts);
  if (!truth.has_value()) {
    debug("No beam electron with a final-state scattered lepton found");
    return;
  }
  if (!truth->beam_hadron.has_value()) {
    debug("No beam hadron found");
    return;
  }

  // Get boost to colinear frame
  static const auto ei_mass = m_particleSvc.particle(11).mass;
  const PxPyPzEVector ei    = four_momentum(truth->beam_lepton(), ei_mass);
  const auto& pi_particle   = *truth->beam_hadron;
  const PxPyPzEVector pi =
      beam_per_nucleon(pi_particle, is_ion(pi_particle.getPDG())
                                        ? 0.
                                        : m_particleSvc.particle(pi_particle.getPDG()).mass)
          .momentum;
  const auto boost = determine_boost(ei, pi);

  // Lepton line and photons radiated from it, excluded from the hadronic final state
  std::vector<podio::ObjectID> excluded;
  for (const auto& lepton : truth->line) {
    excluded.push_back(lepton.getObjectID());
  }
  for (const auto& photon : truth->photons) {
    excluded.push_back(photon.particle.getObjectID());
  }

  // Sum final-state particles of the signal interaction in colinear frame
  PxPyPzEVector sum;
  for (const auto& p : *mcparts) {
    if (p.getGeneratorStatus() != truth->status_base + 1) {
      continue;
    }
    if (std::ranges::find(excluded, p.getObjectID()) != excluded.end()) {
      continue;
    }
    sum += apply_boost(boost, four_momentum(p));
  }

  // Hadronic final state calculations
  const auto [sigma, pT, gamma] = hadronic_final_state_sums(sum);
  auto hfs                      = hadronicfinalstate->create(sigma, pT, gamma);

  debug("sigma_h, pT_h, gamma_h = {},{},{}", hfs.getSigma(), hfs.getPT(), hfs.getGamma());
}

} // namespace eicrecon
