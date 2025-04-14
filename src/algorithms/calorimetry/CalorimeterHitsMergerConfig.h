// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct CalorimeterHitsMergerConfig {

  std::string readout{""};
  std::vector<std::string> fieldTransformations{};
};

} // namespace eicrecon
