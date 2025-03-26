#pragma once

#include <string>

using string = std::string;

namespace eicrecon {

  struct FarForwardHitExtractorConfig {
    double plane1_min_z = 0;
    double plane1_max_z = 0;
    double plane2_min_z = 0;
    double plane2_max_z = 0;
  };

}
