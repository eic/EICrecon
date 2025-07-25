// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>

namespace eicrecon {

struct FarForwardNeutralsReconstructionConfig {
  /** detector constant describing distance to the ZDC */
  std::string offsetPositionName = "HcalFarForwardZDC_SiPMonTile_r_pos";
  /** Correction factors for neutrons in the Hcal */
  std::vector<double> neutronScaleCorrCoeffHcal = {-0.11, -1.5, 0};
  /** Correction factors for gammas in the Hcal */
  std::vector<double> gammaScaleCorrCoeffHcal = {0, -0.13, 0};
  /** rotation from global to local coordinates */
  double globalToProtonRotation = -0.025;
  /** position cuts for the clusters identified as photons */
  double gammaZMaxOffset = 300 * dd4hep::mm;
  /** cuts for the sqrts of the largest and second largest eigenvalues of the moment matrix */
  double gammaMaxLength = 100 * dd4hep::mm;
  double gammaMaxWidth  = 12 * dd4hep::mm;
};

} // namespace eicrecon
