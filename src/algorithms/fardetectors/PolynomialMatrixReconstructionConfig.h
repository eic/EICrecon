// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Alex Jentsch

#pragma once

#include <TGraph2D.h>

namespace eicrecon {

struct PolynomialMatrixConfig {
  // Defaults here are for RPOTS
  double nomMomentum;
};

struct PolynomialMatrixReconstructionConfig {

  float partMass{0.938272};
  float partCharge{1};
  long long partPDG{2212};

  double crossingAngle{0.025};

  std::vector<PolynomialMatrixConfig> poly_matrix_configs;

  double hit1minZ{0};
  double hit1maxZ{0};
  double hit2minZ{0};
  double hit2maxZ{0};

  std::string readout{""};

  bool requireBeamProton{true};
  bool requireValidBeamEnergy{true};
};

} // namespace eicrecon
