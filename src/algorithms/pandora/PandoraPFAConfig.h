// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

/// Configuration for the PandoraPFA-based particle flow algorithm.
struct PandoraPFAConfig {

  /// Path to the Pandora algorithm settings XML file.
  std::string pandoraSettingsFile{"PandoraPFASettings.xml"};

  /// Names of calorimeter hit collections to use as ECAL barrel input.
  std::vector<std::string> ecalBarrelCollections{"EcalBarrelRecHits"};

  /// Names of calorimeter hit collections to use as ECAL endcap input.
  std::vector<std::string> ecalEndcapCollections{"EcalEndcapRecHits"};

  /// Names of calorimeter hit collections to use as HCAL barrel input.
  std::vector<std::string> hcalBarrelCollections{"HcalBarrelRecHits"};

  /// Names of calorimeter hit collections to use as HCAL endcap input.
  std::vector<std::string> hcalEndcapCollections{"HcalEndcapRecHits"};

  /// MIP equivalent energy scale for ECAL hits [GeV/MIP].
  float ecalMipEquivalentEnergy{1.0e-4f};

  /// MIP equivalent energy scale for HCAL hits [GeV/MIP].
  float hcalMipEquivalentEnergy{2.0e-4f};

  /// Sampling fraction for ECAL (used for EM energy calibration).
  float ecalSamplingFraction{1.0f};

  /// Sampling fraction for HCAL (used for hadronic energy calibration).
  float hcalSamplingFraction{1.0f};

}; // end PandoraPFAConfig

} // namespace eicrecon
