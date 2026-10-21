// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>
#include <edm4eic/unit_system.h>

namespace eicrecon {

struct CalorimeterHitsMergerConfig {

  std::string readout{""};
  std::vector<std::string> fieldTransformations{};

  // if > 0, only merge hits that are within this time window (ns)
  double timeWindow{0.0 * edm4eic::unit::ns};
};

} // namespace eicrecon
