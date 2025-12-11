// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <ActsExamples/DD4hepDetector/DD4hepDetector.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace eicrecon {

using VolumeSurfaceMap = std::unordered_map<uint64_t, const Acts::Surface*>;

/// @brief Base class for Acts detector implementations using external DD4hep geometry
///
/// This class inherits from ActsExamples::DD4hepDetectorBase, using the new protected
/// constructor that accepts an external DD4hep detector instead of building from XML.
class ActsDD4hepDetector : public ActsExamples::DD4hepDetectorBase {
public:
  struct Config {
    /// Log level for the geometry service
    Acts::Logging::Level logLevel = Acts::Logging::Level::INFO;

    /// External DD4hep detector
    /// Can be set as either raw pointer (will create non-owning shared_ptr)
    /// or as shared_ptr (will use as-is)
    std::shared_ptr<dd4hep::Detector> dd4hepDetector = nullptr;

    /// Helper to set from const pointer (creates non-owning shared_ptr)
    void setDD4hepDetector(const dd4hep::Detector* detector) {
      if (detector) {
        // Create non-owning shared_ptr with null deleter
        dd4hepDetector = std::shared_ptr<dd4hep::Detector>(const_cast<dd4hep::Detector*>(detector),
                                                           [](dd4hep::Detector*) {}
                                                           // null deleter - we don't own this
        );
      }
    }

    /// The name of the service
    std::string name = "default";

    /// Material decorator (optional)
    std::shared_ptr<const Acts::IMaterialDecorator> materialDecorator = nullptr;

    /// Geometry identifier hook (optional, will use default if not provided)
    std::shared_ptr<const Acts::GeometryIdentifierHook> geometryIdentifierHook = nullptr;

    /// Geometry context
    Acts::GeometryContext geoContext{};
  };

  explicit ActsDD4hepDetector(const Config& cfg);
  virtual ~ActsDD4hepDetector() = default;

  // Inherited from DD4hepDetectorBase:
  // - dd4hep::Detector& dd4hepDetector()
  // - std::shared_ptr<ActsPlugins::DD4hepFieldAdapter> field() const
  // - TGeoNode& tgeoGeometry()

  /// Access to the ACTS tracking geometry
  /// @return The ACTS tracking geometry (implemented by derived classes)
  virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const = 0;

  /// Access to the surface map
  /// @return Map of DD4hep volume IDs to ACTS surfaces
  const VolumeSurfaceMap& surfaceMap() const { return m_surfaces; }

  /// Access to the geometry context
  /// @return The geometry context
  const Acts::GeometryContext& geometryContext() const { return m_cfg.geoContext; }

  /// Access to the logger
  /// @return The logger instance
  const Acts::Logger& logger() const { return *m_logger; }

  /// Required by DD4hepDetectorBase but not used for external detector
  const ActsExamples::DD4hepDetectorBase::Config& config() const override;

protected:
  /// Build the surface map by visiting all surfaces in the tracking geometry
  /// @param trackingGeo The tracking geometry to visit
  void buildSurfaceMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeo);

  /// Configuration
  Config m_cfg;

  /// Logger
  std::unique_ptr<const Acts::Logger> m_logger;

  /// ACTS surface lookup container for hit surfaces
  VolumeSurfaceMap m_surfaces;

private:
  /// Geometry identifier hook implementation
  class GeometryIdentifierHook : public Acts::GeometryIdentifierHook {
  public:
    Acts::GeometryIdentifier decorateIdentifier(Acts::GeometryIdentifier identifier,
                                                const Acts::Surface& surface) const override;
  };

  /// Surface visitor for building the surface map
  void visitSurface(const Acts::Surface* surface);

  /// Dummy config for DD4hepDetectorBase requirement
  ActsExamples::DD4hepDetectorBase::Config m_baseConfig;
};

} // namespace eicrecon
