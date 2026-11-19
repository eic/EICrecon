// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

// Acts_VERSION_MAJOR/MINOR are compile definitions added by plugin_add_acts
#if !defined(Acts_VERSION_MAJOR) || !defined(Acts_VERSION_MINOR)
#error "Acts_VERSION_MAJOR and Acts_VERSION_MINOR must be defined (see plugin_add_acts)"
#endif

// Gen3 (blueprint) geometry needs the DD4hep blueprint builder in ActsPlugins and the blueprint
// classes in the Acts namespace, where they moved from Acts::Experimental in Acts v47.2.0.
// A macro rather than a constexpr, since it guards code that needs the Gen3 headers.
#if __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>) &&                                   \
    (Acts_VERSION_MAJOR > 47 || (Acts_VERSION_MAJOR == 47 && Acts_VERSION_MINOR >= 2))
#define HAS_GEN3_SUPPORT 1 // NOLINT(cppcoreguidelines-macro-usage)
#else
#define HAS_GEN3_SUPPORT 0 // NOLINT(cppcoreguidelines-macro-usage)
#endif

#if HAS_GEN3_SUPPORT

#include <ActsPlugins/Root/TGeoAxes.hpp>
#include <DD4hep/DetElement.h>
#include <functional>
#include <memory>

#include "ActsDD4hepDetector.h"

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
        const dd4hep::DetElement& element, ActsPlugins::TGeoAxes axes, double scale)>;

    ElementFactory detectorElementFactory;
  };

  static std::shared_ptr<ActsPlugins::DD4hepDetectorElement>
  defaultDetectorElementFactory(const dd4hep::DetElement& element, ActsPlugins::TGeoAxes axes,
                                double scale);

  explicit ActsDD4hepDetectorGen3(const Config& cfg);

private:
  void construct();

  Config m_gen3Cfg;
};

} // namespace eicrecon

#endif // HAS_GEN3_SUPPORT
