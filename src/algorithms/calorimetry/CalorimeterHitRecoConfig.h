// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

  struct CalorimeterHitRecoConfig {

    // digitization settings
    unsigned int             capADC{1};
    double                   dyRangeADC{1};
    unsigned int             pedMeanADC{0};
    double                   pedSigmaADC{0};
    double                   resolutionTDC{1};
    double                   corrMeanScale{1};

    // zero suppression
    double                   thresholdFactor{0};
    double                   thresholdValue{0};

    // sampling fraction
    double                   sampFrac{1.0};
    std::vector<double>      sampFracLayer{};

    // readout fields
    std::string              readout{""};
    std::string              layerField{""};
    std::string              sectorField{""};

    // name of detelment or fields to find the local detector (for global->local transform)
    // if nothing is provided, the lowest level DetElement (from cellID) will be used
    std::string              localDetElement{""};
    std::vector<std::string> localDetFields{};
    std::string              maskPos{""};
    std::vector<std::string> maskPosFields{};
  };

} // eicrecon
