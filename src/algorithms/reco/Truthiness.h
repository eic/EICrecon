// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "TruthinessConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TruthinessAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection>,
    algorithms::Output<>>;

class Truthiness : public TruthinessAlgorithm, public WithPodConfig<TruthinessConfig> {

public:
  Truthiness(std::string_view name)
      : TruthinessAlgorithm{
            name,
            {"inputMCParticles", "inputReconstructedParticles", "inputAssociations"},
            {},
            "Calculate truthiness metric comparing reconstructed particles to MC "
            "truth."} {}

  void init() final {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
