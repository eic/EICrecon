// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg, Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"
#include "algorithms/reco/ScatteredElectronsEMinusPzConfig.h"

namespace eicrecon {

using ScatteredElectronsEMinusPzAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class ScatteredElectronsEMinusPz : public ScatteredElectronsEMinusPzAlgorithm,
                                   WithPodConfig<ScatteredElectronsEMinusPzConfig> {

public:
  ScatteredElectronsEMinusPz(std::string_view name)
      : ScatteredElectronsEMinusPzAlgorithm{name,
                                            {"inputParticles", "inputElectronCandidates"},
                                            {"outputElectrons"},
                                            "Outputs DIS electrons ordered in decreasing E-pz"} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
};

} // namespace eicrecon
