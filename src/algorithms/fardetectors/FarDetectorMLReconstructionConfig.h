// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {
  struct FarDetectorMLReconstructionConfig {

    std::string modelPath{"calibrations/tmva/LowQ2_DNN_CPU.weights.xml"};
    std::string methodName{"DNN_CPU"};

    float electronBeamE{10*dd4hep::GeV};

  };
}