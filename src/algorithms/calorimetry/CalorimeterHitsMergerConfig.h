// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

  struct CalorimeterHitsMergerConfig {

    std::string              readout{""};
    std::string              mergeMatrix{""};
    std::vector<std::string> fields{};
    std::vector<int>         refs{};  // TODO might need to evolve this to a vector of vectors...

  };

} // eicrecon
