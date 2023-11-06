// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <string>
#include <vector>

#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {

  struct CalorimeterHitDigiConfig {

    std::vector<double>      eRes;
    double                   tRes;

    // single hit energy deposition threshold
    double                   threshold{1.0*dd4hep::keV};

    // readout settings
    enum readout_enum : int { kSimpleReadout, kPoissonPhotonReadout, kSipmReadout };
    enum readout_enum        readoutType{kSimpleReadout};
    double                   lightYield{0. / dd4hep::GeV};
    double                   photonDetectionEfficiency; // (light collection efficiency) x (quantum efficiency)
    unsigned long long       numEffectiveSipmPixels;

    // digitization settings
    unsigned int             capADC{1};
    double                   capTime{1000}; // dynamic range in ns
    double                   dyRangeADC{1};
    unsigned int             pedMeanADC{0};
    double                   pedSigmaADC{0};
    double                   resolutionTDC{1};
    double                   corrMeanScale{1};

    // signal sums
    std::string              readout{""};
    std::vector<std::string> fields{};

  };

} // eicrecon
