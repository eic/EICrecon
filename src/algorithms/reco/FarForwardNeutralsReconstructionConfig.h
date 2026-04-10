// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>

namespace eicrecon {

struct FarForwardNeutralsReconstructionConfig {
  /** detector constant describing distance to the ZDC */
  std::string offsetPositionName = "HcalFarForwardZDC_SiPMonTile_r_pos";
  /** Correction factors for neutrons in the Hcal (ZDC) */
  std::vector<double> neutronScaleCorrCoeffHcal = {2.4, 0.89};
  /** Correction factors for gammas in the Hcal (ZDC) */
  std::vector<double> gammaScaleCorrCoeffHcal = {1.1, 0.98};
  /** Correction factors for neutrons in the LFHCAL */
  std::vector<double> neutronScaleCorrCoeffLFHCAL = {2.55, 0.95};
  /** Correction factors for gammas in the LFHCAL */
  std::vector<double> gammaScaleCorrCoeffLFHCAL = {0., 0.};
  /** Correction factors for neutrons in the B0-Ecal */
  std::vector<double> neutronScaleCorrCoeffB0Ecal = {0., 0.};
  /** Correction factors for gammas in the B0-Ecal */
  std::vector<double> gammaScaleCorrCoeffB0Ecal = {0.99, 1.14};
  /** Correction factors for neutrons in the Endcap-Ecal */
  std::vector<double> neutronScaleCorrCoeffEcalEndcapP = {0., 0.};
  /** Correction factors for gammas in the Endcap-Ecal */
  std::vector<double> gammaScaleCorrCoeffEcalEndcapP = {1.05, 1.01};
  /** rotation from global to local coordinates */
  double globalToProtonRotation = -0.025;
  /** Neutron-photon separation in HcalFarForwardZDC */
  double gammaZMaxOffset = 400 * dd4hep::mm;
  double gammaMaxLength  = 100 * dd4hep::mm;
  double gammaMaxWidth   = 12 * dd4hep::mm;
};

} // namespace eicrecon
