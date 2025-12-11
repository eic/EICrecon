// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsDD4hepDetector.h"

#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Logger.hpp>
#if __has_include(<ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>)
#include <ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>
#include <ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>
#else
#include <Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp>
#include <Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp>
#endif
#include <DD4hep/Detector.h>
#include <DD4hep/VolumeManager.h>

#include <stdexcept>

namespace eicrecon {

// Ensure ActsPlugins namespace is used when present
#if __has_include(<ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>)
// Acts_MAJOR_VERSION >= 44
using DD4hepDetectorElement = ActsPlugins::DD4hepDetectorElement;
using ActsPlugins::DD4hepFieldAdapter;
#else
// Acts_MAJOR_VERSION < 44
using DD4hepDetectorElement = Acts::DD4hepDetectorElement;
using Acts::DD4hepFieldAdapter;
#endif

// ActsDD4hepDetector implementation
ActsDD4hepDetector::ActsDD4hepDetector(const Config& cfg)
    : m_cfg(cfg), m_logger(Acts::getDefaultLogger(cfg.name, cfg.logLevel)) {

  if (m_cfg.dd4hepDetector == nullptr) {
    throw std::invalid_argument("DD4hep detector pointer cannot be null");
  }

  // Store the DD4hep detector
  m_detector = m_cfg.dd4hepDetector;

  // Set default geometry identifier hook if not provided
  if (m_cfg.geometryIdentifierHook == nullptr) {
    m_cfg.geometryIdentifierHook = std::make_shared<GeometryIdentifierHook>();
  }
}

dd4hep::Detector& ActsDD4hepDetector::dd4hepDetector() {
  if (!m_detector) {
    throw std::runtime_error("DD4hep detector not initialized");
  }
  return *m_detector;
}

std::shared_ptr<DD4hepFieldAdapter> ActsDD4hepDetector::field() const {
  if (!m_detector) {
    throw std::runtime_error("DD4hep detector not initialized");
  }
  return std::make_shared<DD4hepFieldAdapter>(m_detector->field());
}

TGeoNode& ActsDD4hepDetector::tgeoGeometry() {
  if (!m_detector) {
    throw std::runtime_error("DD4hep detector not initialized");
  }
  return m_detector->world().placement();
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
#if Acts_VERSION_MAJOR >= 45
  const auto* placement   = surface->surfacePlacement();
  const auto* det_element = dynamic_cast<const DD4hepDetectorElement*>(placement);
#else
  const auto* det_element =
      dynamic_cast<const DD4hepDetectorElement*>(surface->associatedDetectorElement());
#endif

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

#if Acts_VERSION_MAJOR >= 45
  const auto* placement          = surface.surfacePlacement();
  const auto* dd4hep_det_element = dynamic_cast<const DD4hepDetectorElement*>(placement);
#else
  const auto* dd4hep_det_element =
      dynamic_cast<const DD4hepDetectorElement*>(surface.associatedDetectorElement());
#endif

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
