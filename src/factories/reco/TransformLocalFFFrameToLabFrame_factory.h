// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Alex Jentsch

#pragma once

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "algorithms/reco/TransformLocalFFFrameToLabFrame.h"

namespace eicrecon {

class TransformLocalFFFrameToLabFrame_factory
    : public JOmniAlgorithmFactory<TransformLocalFFFrameToLabFrame_factory,
                                   eicrecon::TransformLocalFFFrameToLabFrame,
                                   TransformLocalFFFrameToLabFrameConfig> {
private:
  ParameterRef<double> m_crossing_angle{this, "crossing_angle", config().m_crossing_angle};
};

} // namespace eicrecon
