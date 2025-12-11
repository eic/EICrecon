// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#if __has_include(<ActsPlugins/DD4hep/ConvertDD4hepDetector.hpp>)
#include <ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>
#else
#include <Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp>
#endif
#include <DD4hep/Detector.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace eicrecon {

// Ensure ActsPlugins namespace is used when present
#if __has_include(<ActsPlugins/DD4hep/ConvertDD4hepDetector.hpp>)
// Acts_MAJOR_VERSION >= 44
using ActsPlugins::DD4hepFieldAdapter;
#else
// Acts_MAJOR_VERSION < 44
using Acts::DD4hepFieldAdapter;
#endif

using VolumeSurfaceMap = std::unordered_map<uint64_t, const Acts::Surface*>;

/// @brief Base class for Acts detector implementations using external DD4hep geometry
///
/// This class provides access to the ACTS tracking geometry, DD4hep detector,
/// magnetic field, and related contexts. Derived classes (Gen1, Gen3) implement
/// generation-specific geometry construction.
class ActsDD4hepDetector {
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
  };

  explicit ActsDD4hepDetector(const Config& cfg);
  virtual ~ActsDD4hepDetector() = default;

  /// @brief Returns the valid pointer to the tracking geometry.
  ///        Throws an exception if the pointer is invalid
  virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const {
    if (m_trackingGeometry == nullptr) {
      throw std::runtime_error("Tracking geometry is not built");
    }
    return m_trackingGeometry;
  }

  /// Interface method to access to the DD4hep geometry
  dd4hep::Detector& dd4hepDetector();

  /// @brief Access to the DD4hep field
  /// @return a shared pointer to the DD4hep field
  std::shared_ptr<DD4hepFieldAdapter> field() const;

  /// Interface method to Access the TGeo geometry
  /// @return The world TGeoNode (physical volume)
  TGeoNode& tgeoGeometry();

  /// @brief Returns the reference to the geometry context
  const Acts::GeometryContext& getActsGeometryContext() const { return m_trackingGeoCtx; }

  /// @brief Returns the reference to the magnetic field context
  const Acts::MagneticFieldContext& getActsMagneticFieldContext() const {
    return m_magneticFieldCtx;
  }

  /// @brief Returns the reference to the calibration context
  const Acts::CalibrationContext& getActsCalibrationContext() const { return m_calibrationCtx; }

  /// Access to the surface map
  /// @return Map of DD4hep volume IDs to ACTS surfaces
  const VolumeSurfaceMap& surfaceMap() const { return m_surfaces; }

protected:
  const Acts::Logger& logger() const { return *m_logger; }

  /// Build the surface map by visiting all surfaces in the tracking geometry
  /// @param trackingGeo The tracking geometry to visit
  void buildSurfaceMap(std::shared_ptr<const Acts::TrackingGeometry> trackingGeo);

  /// Configuration
  Config m_cfg;

  /// Logger
  std::unique_ptr<const Acts::Logger> m_logger;

  /// Pointer to the interface to the DD4hep geometry
  std::shared_ptr<dd4hep::Detector> m_detector;

  /// Tracking geometry
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;

  /// Tracking geometry context
#if Acts_VERSION_MAJOR >= 45
  Acts::GeometryContext m_trackingGeoCtx = Acts::GeometryContext::dangerouslyDefaultConstruct();
#else
  Acts::GeometryContext m_trackingGeoCtx;
#endif

  /// Magnetic field context
  Acts::MagneticFieldContext m_magneticFieldCtx;

  /// Calibration context
  Acts::CalibrationContext m_calibrationCtx;

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
};

} // namespace eicrecon
