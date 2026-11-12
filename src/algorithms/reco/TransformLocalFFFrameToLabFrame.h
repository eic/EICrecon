// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Alex Jentsch
//

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>
#include <string_view>

#include "TransformLocalFFFrameToLabFrameConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TransformLocalFFFrameToLabFrameAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class TransformLocalFFFrameToLabFrame
    : public TransformLocalFFFrameToLabFrameAlgorithm,
      public WithPodConfig<TransformLocalFFFrameToLabFrameConfig> {

public:
  TransformLocalFFFrameToLabFrame(std::string_view name)
      : TransformLocalFFFrameToLabFrameAlgorithm{
            name,
            {"inputFFRecoParticles"},
            {"outputReconstructedParticles"},
            "Transform reconstructed particles in FF local frame by rotation."} {}

  void process(const Input&, const Output&) const final;
};
} // namespace eicrecon
