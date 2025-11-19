// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>

namespace eicrecon {

struct FarForwardSigma0ReconstructionConfig {

  /** maximum deviation between reconstructed mass and PDG mass */
  double sigma0MaxMassDev = 0.070 * dd4hep::GeV;

};

} // namespace eicrecon
