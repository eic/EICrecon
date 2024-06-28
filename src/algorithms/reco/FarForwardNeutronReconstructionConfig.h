// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

namespace eicrecon {

  struct FarForwardNeutronReconstructionConfig {
    std::vector<double>      scale_corr_coeff_hcal={-0.0756, -1.91,  2.30};
    std::vector<double>      scale_corr_coeff_ecal={-0.0756, -1.91,  2.30};
  };

} // eicrecon
