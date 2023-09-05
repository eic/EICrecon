// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {
  struct FarDetectorMLReconstructionConfig {

    std::string modelPath{""};
    std::string methodName{"DNN_CPU"};
    std::string fileName{"LowQ2_DNN_CPU.weights.xml"};
    std::string environmentPath{"JANA_PLUGIN_PATH"};

  };
}
