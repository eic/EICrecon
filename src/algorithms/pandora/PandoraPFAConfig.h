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

  // ====================================================================
  // Optional algorithm parameter overrides (0 or negative = use XML value)
  // ====================================================================

  /// Topological clustering: Maximum separation between calorimeter hits [mm].
  /// Overrides TopologicalAssociationMaxCaloHitSeparation in XML (default 10).
  float topologicalMaxCaloHitSeparation{0};

  /// Topological clustering: Maximum separation between clusters [mm].
  /// Overrides TopologicalAssociationMaxClusterSeparation in XML (default 100).
  float topologicalMaxClusterSeparation{0};

  /// Topological clustering: Maximum cluster opening angle (cos) [dimensionless].
  /// Overrides TopologicalAssociationMaxClusterCosAngle in XML (default 0.95).
  float topologicalMaxClusterCosAngle{0};

  /// Track-cluster association: Maximum calorimeter hit separation [mm].
  /// Overrides TrackClusterAssociationMaxCaloHitSeparation in XML (default 100).
  float trackClusterMaxCaloHitSeparation{0};

  /// Track-cluster association: Maximum cluster separation from track [mm].
  /// Overrides TrackClusterAssociationMaxSeparationFromTrack in XML (default 50).
  float trackClusterMaxSeparationFromTrack{0};

  /// Neutral PFO creation: Minimum cluster energy threshold [GeV].
  /// Overrides NeutralPfoCreation/MinClusterEnergy in XML (default 0.1).
  float neutralPfoMinClusterEnergy{0};

  // ====================================================================
  // Arbor-specific parameter overrides (0 or negative = use XML value)
  // ====================================================================

  /// Arbor clustering: Cell energy threshold for removal [GeV].
  /// Overrides Arbor/CellThresholdForRemoval in XML (default 0.1).
  float arborCellThresholdForRemoval{0};

  /// Arbor clustering: Maximum search layer for tree building [count].
  /// Overrides Arbor/MaxSearchLayer in XML (default 100).
  float arborMaxSearchLayer{0};

  /// Arbor clustering: Maximum transverse cell length multiplier [dimensionless].
  /// Overrides Arbor/MaxTransverseCellLengthMultiplier in XML (default 1.5).
  float arborMaxTransverseCellLengthMultiplier{0};

  /// Arbor clustering: Should merge isolated trees (0 = false, 1 = true).
  /// Overrides Arbor/ShouldMergeIsolatedTrees in XML (default 1 = true).
  int arborShouldMergeIsolatedTrees{-1};

  /// Arbor clustering: Isolated tree energy cut for merging [GeV].
  /// Overrides Arbor/IsolatedTreeEnergyCutForMerging in XML (default 10.0).
  float arborIsolatedTreeEnergyCutForMerging{0};

  /// Arbor clustering: Minimum cluster energy for merging [GeV].
  /// Overrides Arbor/MinClusterEnergyForMerging in XML (default 0.5).
  float arborMinClusterEnergyForMerging{0};

  /// Arbor clustering: Use shower profile (0 = false, 1 = true).
  /// Overrides Arbor/UseShowerProfile in XML (default 1 = true).
  int arborUseShowerProfile{-1};

}; // end PandoraPFAConfig

} // namespace eicrecon
