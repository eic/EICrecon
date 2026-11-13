// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

#pragma once

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <algorithms/algorithm.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <cmath>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

/// Summed quantities of the hadronic final state
struct HadronicFinalStateSums {
  double sigma;
  double pT;
  double gamma;
};

/// Summed quantities of the hadronic final state, from the sum of its four-momenta in the
/// head-on frame
inline HadronicFinalStateSums hadronic_final_state_sums(const ROOT::Math::PxPyPzEVector& sum) {
  const auto sigma = sum.E() - sum.Pz();
  const auto pT    = sum.Pt();
  const auto gamma = std::acos((pT * pT - sigma * sigma) / (pT * pT + sigma * sigma));
  return {sigma, pT, gamma};
}

using HadronicFinalStateAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4hep::MCParticleCollection,
                      edm4eic::ReconstructedParticleCollection,
                      edm4eic::ReconstructedParticleCollection>,
    algorithms::Output<edm4eic::HadronicFinalStateCollection>>;

/// Hadronic final state: all input particles except the scattered electron, the first particle in
/// the scatteredElectron input. The scattered electron is matched to the input particles by object
/// or by a shared track, so it may be selected from the charged particles before these were matched
/// to clusters. Without scattered electron (e.g. in charged-current events), all input particles
/// are included.
class HadronicFinalState : public HadronicFinalStateAlgorithm, public WithPodConfig<NoConfig> {

public:
  HadronicFinalState(std::string_view name)
      : HadronicFinalStateAlgorithm{
            name,
            {"MCBeamElectrons", "MCBeamHadrons", "scatteredElectron", "inputParticles"},
            {"hadronicFinalState"},
            "Calculate summed quantities of the hadronic final state."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_crossingAngle{-0.025};
};

} // namespace eicrecon
