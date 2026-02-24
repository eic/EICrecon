// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

// This class is only available if ActsPlugins with DD4hep support is available,
// and has blueprint builder support (gen3).
#if __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>)

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <DD4hep/DetElement.h>

#include <functional>
#include <memory>
#include <string>

#include "ActsDD4hepDetector.h"

namespace Acts {
class GeometryContext;
}

namespace ActsPlugins {
class DD4hepDetectorElement;
}

namespace eicrecon {

/// @brief Gen3 DD4hep detector implementation using Blueprint builder
///
/// This implementation uses the modern Blueprint-based geometry construction
/// which allows for explicit definition of the detector hierarchy.
class ActsDD4hepDetectorGen3 final : public ActsDD4hepDetector {
public:
  struct Config : public ActsDD4hepDetector::Config {
    using ElementFactory = std::function<std::shared_ptr<ActsPlugins::DD4hepDetectorElement>(
        const dd4hep::DetElement& element, const std::string& axes, double scale)>;

    ElementFactory detectorElementFactory;
  };

  static std::shared_ptr<ActsPlugins::DD4hepDetectorElement>
  defaultDetectorElementFactory(const dd4hep::DetElement& element, const std::string& axes,
                                double scale);

  explicit ActsDD4hepDetectorGen3(const Config& cfg);

private:
  void construct();

  Config m_gen3Cfg;
};

} // namespace eicrecon

#endif // __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>)
