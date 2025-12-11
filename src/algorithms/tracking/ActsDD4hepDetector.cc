// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsDD4hepDetector.h"

#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>
#include <DD4hep/Detector.h>
#include <DD4hep/VolumeManager.h>

#include <stdexcept>

namespace eicrecon {

ActsDD4hepDetector::ActsDD4hepDetector(const Config& cfg)
    : ActsExamples::DD4hepDetectorBase(cfg.dd4hepDetector, cfg.logLevel)
    , m_cfg(cfg)
    , m_logger(Acts::getDefaultLogger(cfg.name, cfg.logLevel)) {

  if (m_cfg.dd4hepDetector == nullptr) {
    throw std::invalid_argument("DD4hep detector pointer cannot be null");
  }

  // Set default geometry identifier hook if not provided
  if (m_cfg.geometryIdentifierHook == nullptr) {
    m_cfg.geometryIdentifierHook = std::make_shared<GeometryIdentifierHook>();
  }

  // Initialize base config for the required config() method
  m_baseConfig.name     = cfg.name;
  m_baseConfig.logLevel = cfg.logLevel;
}

const ActsExamples::DD4hepDetectorBase::Config& ActsDD4hepDetector::config() const {
  return m_baseConfig;
}

void ActsDD4hepDetector::buildSurfaceMap(
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeo) {

  if (!trackingGeo) {
    logger().log(Acts::Logging::WARNING, "Cannot build surface map: tracking geometry is null");
    return;
  }

  m_surfaces.clear();

  // Visit all surfaces and build the map
  trackingGeo->visitSurfaces([this](const Acts::Surface* surface) { this->visitSurface(surface); });

  logger().log(Acts::Logging::INFO,
               "Built surface map with " + std::to_string(m_surfaces.size()) + " surfaces");
}

void ActsDD4hepDetector::visitSurface(const Acts::Surface* surface) {
  // Require a valid surface
  if (surface == nullptr) {
    return;
  }

  // Get the DD4hep detector element
  const auto* det_element =
      dynamic_cast<const ActsPlugins::DD4hepDetectorElement*>(surface->associatedDetectorElement());

  if (det_element == nullptr) {
    return;
  }

  // Look up the volume ID from DD4hep
  auto volman   = dd4hepDetector().volumeManager();
  auto* vol_ctx = volman.lookupContext(det_element->identifier());
  auto vol_id   = vol_ctx->identifier;

  // Add to the surface map
  m_surfaces.insert_or_assign(vol_id, surface);
}

Acts::GeometryIdentifier
ActsDD4hepDetector::GeometryIdentifierHook::decorateIdentifier(Acts::GeometryIdentifier identifier,
                                                               const Acts::Surface& surface) const {

  const auto* dd4hep_det_element =
      dynamic_cast<const ActsPlugins::DD4hepDetectorElement*>(surface.associatedDetectorElement());

  if (dd4hep_det_element == nullptr) {
    return identifier;
  }

  // Set 8-bit extra field to 8-bit DD4hep detector ID
#if Acts_VERSION_MAJOR >= 40
  return identifier.withExtra(0xff & dd4hep_det_element->identifier());
#else
  return identifier.setExtra(0xff & dd4hep_det_element->identifier());
#endif
}

} // namespace eicrecon
