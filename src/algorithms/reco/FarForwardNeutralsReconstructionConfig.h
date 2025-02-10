// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>


namespace eicrecon {

  struct FarForwardNeutralsReconstructionConfig {
    /** Correction factors for neutrons in the Hcal */
    std::vector<double>      n_scale_corr_coeff_hcal={-0.11, -1.5, 0};
    /** Correction factors for gammas in the Hcal */
    std::vector<double>      gamma_scale_corr_coeff_hcal={0, -0.13, 0};
    /** rotation from global to local coordinates */
    double rot_y=-0.025;
    /** position cuts for the clusters identified as photons */
    double gamma_zmax=(300+35800)*dd4hep::mm;
    /** cuts for the sqrts of the largest and second largest eigenvalues of the moment matrix */
    double gamma_max_length=100*dd4hep::mm;
    double gamma_max_width=12*dd4hep::mm;
  };

} // eicrecon
