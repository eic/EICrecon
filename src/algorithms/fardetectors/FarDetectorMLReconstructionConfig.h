// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  struct FarDetectorMLReconstructionConfig {

    std::string modelPath{""};
    std::string methodName{"DNN_CPU"};
    std::string fileName{"LowQ2_DNN_CPU.weights.xml"};
    std::string environmentPath{"JANA_PLUGIN_PATH"};

  };
}
