// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

namespace eicrecon {

  struct FarForwardNeutronReconstructionConfig {
    /** Correction factors for the Hcal */
    std::vector<double>      scale_corr_coeff_hcal={-0.0756, -1.91, 2.30};
    /** Correction factors for the (optional) Ecal */
    std::vector<double>      scale_corr_coeff_ecal={-0.352, -1.34, 1.61};
  };

} // eicrecon
