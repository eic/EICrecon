// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Daniel Brandenburg, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>
#include <string_view>

#include "ElectronReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ElectronReconstructionAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class ElectronReconstruction : public ElectronReconstructionAlgorithm,
                               public WithPodConfig<ElectronReconstructionConfig> {

public:
  ElectronReconstruction(std::string_view name)
      : ElectronReconstructionAlgorithm{name,
                                        {"inputParticles"},
                                        {"outputParticles"},
                                        "selected electrons from reconstructed particles"} {}

  void init() final {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
