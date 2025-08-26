// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

namespace eicrecon {

struct ElectronReconstructionConfig {

  double min_energy_over_momentum = 0.9;
  double max_energy_over_momentum = 1.2;
};

} // namespace eicrecon
