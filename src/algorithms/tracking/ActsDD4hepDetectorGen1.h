// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/Definitions/Units.hpp>
#include <Acts/Utilities/BinningType.hpp>
#include <DD4hep/DetElement.h>
#include <functional>
#include <vector>

#include "ActsDD4hepDetector.h"

namespace eicrecon {

/// @brief Gen1 DD4hep detector implementation using auto-detection
///
/// This implementation uses the legacy DD4hep plugin conversion that
/// automatically detects layers and volumes based on DD4hep structure.
class ActsDD4hepDetectorGen1 final : public ActsDD4hepDetector {
public:
  struct Config : public ActsDD4hepDetector::Config {
    /// Binning type in phi
    Acts::BinningType bTypePhi = Acts::equidistant;
    /// Binning type in r
    Acts::BinningType bTypeR = Acts::arbitrary;
    /// Binning type in z
    Acts::BinningType bTypeZ = Acts::equidistant;

    /// The tolerance added to the geometrical extension in r
    double layerEnvelopeR = 1 * Acts::UnitConstants::mm;

    /// The tolerance added to the geometrical extension in z
    double layerEnvelopeZ = 1 * Acts::UnitConstants::mm;

    /// Default layer thickness (for layers without explicit thickness)
    double defaultLayerThickness = 1e-10;

    /// Function to sort detector elements
    std::function<void(std::vector<dd4hep::DetElement>&)> sortDetectors = nullptr;
  };

  explicit ActsDD4hepDetectorGen1(const Config& cfg);

private:
  void construct();

  Config m_gen1Cfg;
};

} // namespace eicrecon
