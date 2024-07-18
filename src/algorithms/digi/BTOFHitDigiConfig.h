// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

  struct BTOFHitDigiConfig {
      // single hit energy deposition threshold
    double threshold{1.0*dd4hep::keV};
    double tRes = 0.1;   /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
      // digitization settings
    //unsigned int             pedMeanADC{0};
    //double                   pedSigmaADC{0};
    double                   resolutionTDC{1};
    double                   resolutionADC{1};
  };

} // eicrecon
