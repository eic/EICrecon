// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

  struct CalorimeterHitDigiConfig {

    std::vector<double>      eRes;
    double                   tRes;

    // single hit energy deposition threshold
    double                   threshold{0};

    // digitization settings
    unsigned int             capADC{1};
    double                   capTime{1}; // FIXME unsigned int?
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
