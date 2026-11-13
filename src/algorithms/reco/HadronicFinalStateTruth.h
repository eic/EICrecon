// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using HadronicFinalStateTruthAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4eic::HadronicFinalStateCollection>>;

/// Generator-level hadronic final state of the signal interaction: all final-state particles of
/// the signal interaction (including neutrinos), except the lepton line and the real photons
/// radiated from it, i.e. the final state of the hadronic vertex, P + q with q as in
/// InclusiveKinematicsTruthBorn. Summed in the head-on frame of the beams of the signal
/// interaction. The hadrons relation is left empty, since it refers to reconstructed particles.
class HadronicFinalStateTruth : public HadronicFinalStateTruthAlgorithm,
                                public WithPodConfig<NoConfig> {

public:
  HadronicFinalStateTruth(std::string_view name)
      : HadronicFinalStateTruthAlgorithm{
            name,
            {"MCParticles"},
            {"hadronicFinalState"},
            "Calculate summed quantities of the generator-level hadronic final state."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
};

} // namespace eicrecon
