// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/onnx/InclusiveKinematicsML.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class InclusiveKinematicsML_factory
    : public JOmniAlgorithmFactory<InclusiveKinematicsML_factory, eicrecon::InclusiveKinematicsML,
                                   InclusiveKinematicsMLConfig> {
private:
  ParameterRef<std::string> m_modelPath{this, "modelPath", config().modelPath};
};

} // namespace eicrecon
