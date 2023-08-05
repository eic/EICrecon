// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

  struct ImagingPixelRecoConfig {

    // digitization settings
    unsigned int             capADC{1};
    double                   dyRangeADC{1};
    unsigned int             pedMeanADC{0};
    double                   pedSigmaADC{0};

    // zero suppression
    double                   thresholdFactor{0};
    double                   thresholdValue{0};

    // sampling fraction
    double                   sampFrac{1.0};

    // readout fields
    std::string              readout{""};
    std::string              layerField{""};
    std::string              sectorField{""};

  };

} // eicrecon
