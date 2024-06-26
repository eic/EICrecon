// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <string>

namespace eicrecon {

  struct ChargeSharingDigitizationMLConfig {

    std::string readout{"TaggerTrackerHits"};
    double timeBinning{0.195 * dd4hep::ns};

    std::string x_field{"x"};
    std::string y_field{"y"};
    std::string sensor_field{"sensor"};
    std::string layer_field{"layer"};
    std::string module_field{"module"};
    std::string detector_field{"system"};

    std::string modelPath{"calibrations/onnx/model_digitization_genprop.onnx"};

    int symmetry{1};
    int segment{1};

  };

} // eicrecon
