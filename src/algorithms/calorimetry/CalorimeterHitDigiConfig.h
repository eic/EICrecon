// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

  struct CalorimeterHitDigiConfig {

    std::vector<double>      eRes;
    double                   tRes;

    // single hit energy deposition threshold
    double                   threshold{1.0*dd4hep::keV};

    // digitization settings
    unsigned int             capADC{1};
    double                   capTime{1000}; // dynamic range in ns
    double                   dyRangeADC{1};
    unsigned int             pedMeanADC{0};
    double                   pedSigmaADC{0};
    double                   resolutionTDC{1};
    double                   corrMeanScale{1};

    // attenuation and light collection efficiency
    bool                     reversedDir{false};
    double                   attenuation{0.0};   // per meter
    double                   collectionEff{1.0}; // light collection efficiency at end

    // signal sums
    std::string              readout{""};
    std::vector<std::string> fields{};

  };

} // eicrecon
