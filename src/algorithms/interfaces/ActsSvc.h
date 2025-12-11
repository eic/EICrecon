// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dmitry Kalinkin

#pragma once

#include <algorithms/logger.h>
#include <algorithms/service.h>
#include <memory>
#include <string>

// Forward declarations
namespace dd4hep {
class Detector;
}

namespace eicrecon {
class ActsDD4hepDetector;
}

namespace algorithms {

class ActsSvc : public LoggedService<ActsSvc> {
public:
  // Default init (no-op, for use with setInit lambda)
  void init() {}

  // Initialize from DD4hep detector (creates ActsDD4hepDetector internally)
  void init(const dd4hep::Detector* dd4hep_detector);

  // Initialize from pre-created ActsDD4hepDetector (for backward compatibility)
  void init(std::shared_ptr<const eicrecon::ActsDD4hepDetector> acts_detector);

  // Initialize with failure
  void init(std::exception_ptr&& _failure);

  std::shared_ptr<const eicrecon::ActsDD4hepDetector> detector() const {
    if (failure) {
      std::rethrow_exception(failure);
    }
    return m_acts_detector;
  }

private:
  // Configuration Properties
  Property<std::string> m_materialMap{this, "MaterialMap", "calibrations/materials-map.cbor",
                                      "JSON/CBOR material map file path (empty to disable)"};

  Property<int> m_generation{
      this, "Generation", 1,
      "Acts geometry generation (1=auto-detect, 3=blueprint; Gen3 requires Acts v36+)"};

  Property<std::string> m_detectorName{this, "DetectorName", "EIC", "Name of the detector"};

  Property<double> m_layerEnvelopeR{this, "LayerEnvelopeR", 1.0,
                                    "Layer envelope in R (mm) for Gen1"};

  Property<double> m_layerEnvelopeZ{this, "LayerEnvelopeZ", 1.0,
                                    "Layer envelope in Z (mm) for Gen1"};

  Property<double> m_defaultLayerThickness{this, "DefaultLayerThickness", 1e-10,
                                           "Default layer thickness (mm) for Gen1"};

protected:
  std::shared_ptr<const eicrecon::ActsDD4hepDetector> m_acts_detector{nullptr};
  std::exception_ptr failure;

  ALGORITHMS_DEFINE_LOGGED_SERVICE(ActsSvc)
};

} // namespace algorithms
