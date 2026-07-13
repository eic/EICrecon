// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>
#include <string>
#include <vector>

namespace eicrecon {

struct FarForwardNeutralsReconstructionConfig {
  /** detector constant describing distance reference position */
  std::string offsetPositionName = "HcalFarForwardZDC_SiPMonTile_r_pos";

  /** Correction factors */
  std::vector<double> neutronScaleCorrCoeff = {0.0, 0.0};
  std::vector<double> gammaScaleCorrCoeff   = {0.0, 0.0};

  /** Detector capabilities */
  bool canDetectGammas   = true;
  bool canDetectNeutrons = true;

  /** Reconstruction modes */
  std::string gammaMode   = "None";
  std::string neutronMode = "None";

  /** Cluster threshold */
  double clusterEmin = 0.0; // GeV

  /** Whether all non-gamma clusters are summed into one neutron candidate */
  bool associateAllClustersToNeutron = false;

  /** rotation from global to local coordinates */
  double globalToProtonRotation = -0.025;

  /** Neutron-photon separation used for ZDC */
  double gammaZMaxOffset = 400;
  double gammaMaxLength  = 100;
  double gammaMaxWidth   = 12;
  double gammaMaxNhitsCoeffLin = 0.3;
  double gammaMaxNhitsCoeffSqrt = 30;
};

} // namespace eicrecon
