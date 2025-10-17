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

double calculateOffsetFromXL(int whichOffset, double x_L, double beamEnergy) {

  if (whichOffset > 4) {
    throw std::runtime_exception(fmt::format("Bad offset index {}", whichOffset));
  }

  double offset_value_and_par[4][3];

  //because of the optics configuration, these 6 parameters are zero
  //but this may not always be true, so we leave these parameters here
  //to account for x-y coupling with different optics or for the OMD

  offset_value_and_par[2][0] = 0.0;
  offset_value_and_par[2][1] = 0.0;
  offset_value_and_par[2][2] = 0.0;
  offset_value_and_par[3][0] = 0.0;
  offset_value_and_par[3][1] = 0.0;
  offset_value_and_par[3][2] = 0.0;

  if (beamEnergy == 275) { //275 GeV
    offset_value_and_par[0][0] = -1916.507316;
    offset_value_and_par[0][1] = 11.100930;
    offset_value_and_par[0][2] = -0.040338;
    offset_value_and_par[1][0] = -84.913431;
    offset_value_and_par[1][1] = 0.614101;
    offset_value_and_par[1][2] = -0.002200;
  } else if (beamEnergy == 100) { //100 GeV
    offset_value_and_par[0][0] = -1839.212116;
    offset_value_and_par[0][1] = 9.573747;
    offset_value_and_par[0][2] = -0.032770;
    offset_value_and_par[1][0] = -80.659006;
    offset_value_and_par[1][1] = 0.530685;
    offset_value_and_par[1][2] = -0.001790;
  } else if (beamEnergy == 130) { //130 GeV
    offset_value_and_par[0][0] = -1875.115783;
    offset_value_and_par[0][1] = 10.291006;
    offset_value_and_par[0][2] = -0.036341;
    offset_value_and_par[1][0] = -82.577631;
    offset_value_and_par[1][1] = 0.567990;
    offset_value_and_par[1][2] = -0.001972;
  } else if (beamEnergy == 41) { //41 GeV
    offset_value_and_par[0][0] = -1754.718344;
    offset_value_and_par[0][1] = 7.767054;
    offset_value_and_par[0][2] = -0.023105;
    offset_value_and_par[1][0] = -73.956525;
    offset_value_and_par[1][1] = 0.391292;
    offset_value_and_par[1][2] = -0.001063;
  } else
    throw std::runtime_exception(fmt::format("Unknown beamEnergy {}", beamEnergy));

  return (offset_value_and_par[whichOffset][0] + offset_value_and_par[whichOffset][1] * x_L +
          offset_value_and_par[whichOffset][2] * x_L * x_L);
}

double calculateMatrixValueFromXL(int whichElement, double x_L, double beamEnergy) {

  double matrix_value_and_par[8][3];

  if ((whichElement < 0) || (whichElement > 7)) {
    throw std::runtime_exception(fmt::format("Bad Array index(ces)", whichElement));
  }

  if (beamEnergy == 275) { //275 GeV
    matrix_value_and_par[0][0] = -94.860289;
    matrix_value_and_par[0][1] = 2.373011;
    matrix_value_and_par[0][2] = -0.011253;
    matrix_value_and_par[1][0] = 4.029769;
    matrix_value_and_par[1][1] = 0.011753;
    matrix_value_and_par[1][2] = -0.000198;
    matrix_value_and_par[2][0] = -8.278798;
    matrix_value_and_par[2][1] = 0.157343;
    matrix_value_and_par[2][2] = -0.000728;
    matrix_value_and_par[3][0] = 0.217521;
    matrix_value_and_par[3][1] = 0.000788;
    matrix_value_and_par[3][2] = -0.000011;
    matrix_value_and_par[4][0] = -0.224777;
    matrix_value_and_par[4][1] = 0.003369;
    matrix_value_and_par[4][2] = -0.000015;
    matrix_value_and_par[5][0] = -162.881282;
    matrix_value_and_par[5][1] = 2.959217;
    matrix_value_and_par[5][2] = -0.013032;
    matrix_value_and_par[6][0] = -0.006100;
    matrix_value_and_par[6][1] = 0.000045;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -7.781919;
    matrix_value_and_par[7][1] = 0.138049;
    matrix_value_and_par[7][2] = -0.000618;
  } else if (beamEnergy == 100) { //100 GeV
    matrix_value_and_par[0][0] = -145.825102;
    matrix_value_and_par[0][1] = 3.099320;
    matrix_value_and_par[0][2] = -0.014369;
    matrix_value_and_par[1][0] = 1.600321;
    matrix_value_and_par[1][1] = 0.055970;
    matrix_value_and_par[1][2] = -0.000407;
    matrix_value_and_par[2][0] = -10.874678;
    matrix_value_and_par[2][1] = 0.193778;
    matrix_value_and_par[2][2] = -0.000883;
    matrix_value_and_par[3][0] = 0.188975;
    matrix_value_and_par[3][1] = 0.000745;
    matrix_value_and_par[3][2] = -0.000008;
    matrix_value_and_par[4][0] = -0.341312;
    matrix_value_and_par[4][1] = 0.005630;
    matrix_value_and_par[4][2] = -0.000026;
    matrix_value_and_par[5][0] = -193.516983;
    matrix_value_and_par[5][1] = 3.532786;
    matrix_value_and_par[5][2] = -0.015695;
    matrix_value_and_par[6][0] = -0.007973;
    matrix_value_and_par[6][1] = 0.000064;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -7.287608;
    matrix_value_and_par[7][1] = 0.118922;
    matrix_value_and_par[7][2] = -0.000468;
  } else if (beamEnergy == 130) { //130 GeV
    matrix_value_and_par[0][0] = -124.028256;
    matrix_value_and_par[0][1] = 2.790119;
    matrix_value_and_par[0][2] = -0.013056;
    matrix_value_and_par[1][0] = 2.985763;
    matrix_value_and_par[1][1] = 0.029403;
    matrix_value_and_par[1][2] = -0.000275;
    matrix_value_and_par[2][0] = -9.921622;
    matrix_value_and_par[2][1] = 0.181781;
    matrix_value_and_par[2][2] = -0.000838;
    matrix_value_and_par[3][0] = 0.322642;
    matrix_value_and_par[3][1] = -0.002093;
    matrix_value_and_par[3][2] = 0.000007;
    matrix_value_and_par[4][0] = -0.291535;
    matrix_value_and_par[4][1] = 0.004711;
    matrix_value_and_par[4][2] = -0.000022;
    matrix_value_and_par[5][0] = -173.506851;
    matrix_value_and_par[5][1] = 3.174727;
    matrix_value_and_par[5][2] = -0.014032;
    matrix_value_and_par[6][0] = -0.006733;
    matrix_value_and_par[6][1] = 0.000052;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -7.030784;
    matrix_value_and_par[7][1] = 0.118149;
    matrix_value_and_par[7][2] = -0.000485;
  } else if (beamEnergy == 41) { //41 GeV
    matrix_value_and_par[0][0] = -174.583113;
    matrix_value_and_par[0][1] = 3.599373;
    matrix_value_and_par[0][2] = -0.016738;
    matrix_value_and_par[1][0] = -2.768064;
    matrix_value_and_par[1][1] = 0.143176;
    matrix_value_and_par[1][2] = -0.000847;
    matrix_value_and_par[2][0] = -12.409090;
    matrix_value_and_par[2][1] = 0.218275;
    matrix_value_and_par[2][2] = -0.000994;
    matrix_value_and_par[3][0] = -0.135417;
    matrix_value_and_par[3][1] = 0.006663;
    matrix_value_and_par[3][2] = -0.000036;
    matrix_value_and_par[4][0] = -0.299041;
    matrix_value_and_par[4][1] = 0.004490;
    matrix_value_and_par[4][2] = -0.000020;
    matrix_value_and_par[5][0] = -175.144897;
    matrix_value_and_par[5][1] = 3.222149;
    matrix_value_and_par[5][2] = -0.014292;
    matrix_value_and_par[6][0] = -0.007376;
    matrix_value_and_par[6][1] = 0.000052;
    matrix_value_and_par[6][2] = 0.000000;
    matrix_value_and_par[7][0] = -8.443702;
    matrix_value_and_par[7][1] = 0.155002;
    matrix_value_and_par[7][2] = -0.000708;
  } else
    throw std::runtime_exception(fmt::format("Unknown beamEnergy {}", beamEnergy));

  return (matrix_value_and_par[whichElement][0] + matrix_value_and_par[whichElement][1] * x_L +
          matrix_value_and_par[whichElement][2] * x_L * x_L);
}

} // namespace eicrecon
