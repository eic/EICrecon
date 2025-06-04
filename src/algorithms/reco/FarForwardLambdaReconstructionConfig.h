// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>

namespace eicrecon {

struct FarForwardLambdaReconstructionConfig {

  /** detector constant describing distance to the ZDC */
  std::string offsetPositionName = "HcalFarForwardZDC_SiPMonTile_r_pos";
  /** transformation from global coordinates to proton-frame coordinates*/
  double globalToProtonRotation = -0.025;
  /** maximum deviation between reconstructed mass and PDG mass */
  double lambdaMaxMassDev = 0.030 * dd4hep::GeV;
  /** number of iterations for the IDOLA algorithm */
  int iterations = 10;
};

} // namespace eicrecon
