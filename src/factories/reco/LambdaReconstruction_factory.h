// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse

#pragma once

#include "algorithms/reco/LambdaReconstruction.h"
#include "algorithms/reco/LambdaReconstructionConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class LambdaReconstruction_factory
    : public JOmniAlgorithmFactory<LambdaReconstruction_factory, eicrecon::LambdaReconstruction,
                                   LambdaReconstructionConfig> {
private:
  ParameterRef<std::string> m_offset_position_name{this, "offsetPositionName",
                                                   config().offsetPositionName};
  ParameterRef<double> m_rot_y{this, "globalToProtonRotation", config().globalToProtonRotation};
  ParameterRef<double> m_lambda_mass_window{this, "lambdaMassWindow", config().lambdaMassWindow};
  ParameterRef<double> m_pi0_mass_window{this, "pi0Window", config().pi0Window};
  ParameterRef<int> m_iterations{this, "iterations", config().iterations};
};

} // namespace eicrecon
