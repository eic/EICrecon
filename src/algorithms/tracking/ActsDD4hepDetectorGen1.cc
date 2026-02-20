// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsDD4hepDetectorGen1.h"

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Utilities/Logger.hpp>
#if __has_include(<ActsPlugins/DD4hep/ConvertDD4hepDetector.hpp>)
#include <ActsPlugins/DD4hep/ConvertDD4hepDetector.hpp>
#else
#include <Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp>
#endif
#include <DD4hep/Detector.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace eicrecon {

// Ensure ActsPlugins namespace is used when present
#if __has_include(<ActsPlugins/DD4hep/ConvertDD4hepDetector.hpp>)
// Acts_MAJOR_VERSION >= 44
using ActsPlugins::convertDD4hepDetector;
#else
// Acts_MAJOR_VERSION < 44
using Acts::convertDD4hepDetector;
#endif

ActsDD4hepDetectorGen1::ActsDD4hepDetectorGen1(const Config& cfg)
    : ActsDD4hepDetector(cfg), m_gen1Cfg(cfg) {

  logger().log(Acts::Logging::INFO, "ActsDD4hepDetectorGen1 constructing...");
  construct();
  logger().log(Acts::Logging::INFO, "ActsDD4hepDetectorGen1 construction complete");
}

void ActsDD4hepDetectorGen1::construct() {
  logger().log(Acts::Logging::INFO, "Converting DD4hep geometry to ACTS (Gen1)...");

  // Convert DD4hep geometry to ACTS using the gen1 auto-detection approach
  // Note: We explicitly pass the detector element factory to avoid undefined symbol issues
  m_trackingGeometry = convertDD4hepDetector(
      dd4hepDetector().world(), logger(), m_gen1Cfg.bTypePhi, m_gen1Cfg.bTypeR, m_gen1Cfg.bTypeZ,
      m_gen1Cfg.layerEnvelopeR, m_gen1Cfg.layerEnvelopeZ, m_gen1Cfg.defaultLayerThickness,
      m_gen1Cfg.sortDetectors, m_trackingGeoCtx, m_cfg.materialDecorator,
      m_cfg.geometryIdentifierHook);

  if (!m_trackingGeometry) {
    logger().log(Acts::Logging::ERROR, "Failed to convert DD4hep geometry to ACTS");
    throw std::runtime_error("DD4hep to ACTS conversion failed");
  }

  logger().log(Acts::Logging::INFO, "DD4hep geometry converted successfully");

  // Build the surface map
  buildSurfaceMap(m_trackingGeometry);
}

} // namespace eicrecon
