// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Simon Gardner, Dmitry Kalinkin

#pragma once

namespace eicrecon {

struct MatrixConfig {
  // Defaults here are for RPOTS
  double nomMomentum;
  std::vector<std::vector<double>> aX;
  std::vector<std::vector<double>> aY;
  double local_x_offset;
  double local_y_offset;
  double local_x_slope_offset;
  double local_y_slope_offset;
};

struct MatrixTransferStaticConfig {

  float partMass{0.938272};
  float partCharge{1};
  long long partPDG{2212};

  double crossingAngle{0.025};

  std::vector<MatrixConfig> matrix_configs;

  double hit1minZ{0};
  double hit1maxZ{0};
  double hit2minZ{0};
  double hit2maxZ{0};

  std::string readout{""};

  bool requireBeamProton{true};
  bool requireMatchingMatrix{true};
};

} // namespace eicrecon
