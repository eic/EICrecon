// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2015 - 2024 Julia Hrdinka, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

// ACTS
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#if __has_include(<ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>)
#include <ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>
#else
#include <Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp>
#endif
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Visualization/ViewConfig.hpp>
#include <DD4hep/Detector.h>
#include <DD4hep/Fields.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <spdlog/logger.h>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace dd4hep::rec {
class Surface;
}

// Alias DD4hepFieldAdapter for different Acts versions
#if __has_include(<ActsPlugins/DD4hep/DD4hepFieldAdapter.hpp>)
using DD4hepFieldAdapter = ActsPlugins::DD4hepFieldAdapter;
#else
using DD4hepFieldAdapter = Acts::DD4hepFieldAdapter;
#endif

/** Draw the surfaces and save to obj file.
 *  This is useful for debugging the ACTS geometry. The obj file can
 *  be loaded into various tools, such as FreeCAD, for inspection.
 */
void draw_surfaces(std::shared_ptr<const Acts::TrackingGeometry> trk_geo,
                   std::shared_ptr<spdlog::logger> init_log, const std::string& fname);

class ActsGeometryProvider {
public:
  ActsGeometryProvider() {};
  virtual ~ActsGeometryProvider() = default;

  using VolumeSurfaceMap = std::unordered_map<uint64_t, const Acts::Surface*>;

  virtual void initialize(const dd4hep::Detector* dd4hep_geo, std::string material_file,
                          std::shared_ptr<spdlog::logger> log,
                          std::shared_ptr<spdlog::logger> init_log) final;

  const dd4hep::Detector* dd4hepDetector() const { return m_dd4hepDetector; }

  /** Gets the ACTS tracking geometry.
     */
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const { return m_trackingGeo; }

  std::shared_ptr<const Acts::MagneticFieldProvider> getFieldProvider() const;

  double centralMagneticField() const {
    return m_dd4hepDetector->field().magneticField({0, 0, 0}).z() *
           (Acts::UnitConstants::T / dd4hep::tesla);
  }

  const VolumeSurfaceMap& surfaceMap() const { return m_surfaces; }

  std::map<int64_t, dd4hep::rec::Surface*> getDD4hepSurfaceMap() const { return m_surfaceMap; }

  const Acts::GeometryContext& getActsGeometryContext() const { return m_trackingGeoCtx; }

  ///  ACTS general logger that is used for running ACTS
  std::shared_ptr<spdlog::logger> getActsRelatedLogger() const { return m_log; }

  /// Logger that is used for geometry initialization
  /// By default its level the same as ACTS general logger (m_log)
  /// But it might be customized to solely printout geometry information
  std::shared_ptr<spdlog::logger> getActsInitRelatedLogger() const { return m_init_log; }

private:
  /** DD4hep detector interface class.
     * This is the main dd4hep detector handle.
     * <a href="https://dd4hep.web.cern.ch/dd4hep/reference/classdd4hep_1_1Detector.html">See DD4hep Detector documentation</a>
     */
  const dd4hep::Detector* m_dd4hepDetector = nullptr;

  /// DD4hep surface map
  std::map<int64_t, dd4hep::rec::Surface*> m_surfaceMap;

  /// ACTS Tracking Geometry Context
  Acts::GeometryContext m_trackingGeoCtx;

  /// ACTS Tracking Geometry
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeo{nullptr};

  /// ACTS surface lookup container for hit surfaces that generate smeared hits
  VolumeSurfaceMap m_surfaces;

  /// Acts magnetic field
  std::shared_ptr<const DD4hepFieldAdapter> m_magneticField = nullptr;

  ///  ACTS general logger that is used for running ACTS
  std::shared_ptr<spdlog::logger> m_log;

  /// Logger that is used for geometry initialization
  /// By default its level the same as ACTS general logger (m_log)
  /// But it might be customized to solely printout geometry information
  std::shared_ptr<spdlog::logger> m_init_log;

  /// Configuration for obj export
#if Acts_VERSION_MAJOR >= 37
  Acts::ViewConfig m_containerView{.color = {220, 220, 220}}; // alto
  Acts::ViewConfig m_volumeView{.color = {220, 220, 0}};      // barberry yellow
  Acts::ViewConfig m_sensitiveView{.color = {0, 180, 240}};   // picton blue
  Acts::ViewConfig m_passiveView{.color = {240, 180, 0}};     // lightning yellow
  Acts::ViewConfig m_gridView{.color = {220, 0, 0}};          // scarlet red
#else
  Acts::ViewConfig m_containerView{{220, 220, 220}}; // alto
  Acts::ViewConfig m_volumeView{{220, 220, 0}};      // barberry yellow
  Acts::ViewConfig m_sensitiveView{{0, 180, 240}};   // picton blue
  Acts::ViewConfig m_passiveView{{240, 180, 0}};     // lightning yellow
  Acts::ViewConfig m_gridView{{220, 0, 0}};          // scarlet red
#endif
  bool m_objWriteIt{false};
  bool m_plyWriteIt{false};
  std::string m_outputTag{""};
  std::string m_outputDir{""};

public:
  void setObjWriteIt(bool writeit) { m_objWriteIt = writeit; }
  bool getObjWriteIt() const { return m_objWriteIt; }
  void setPlyWriteIt(bool writeit) { m_plyWriteIt = writeit; }
  bool getPlyWriteIt() const { return m_plyWriteIt; }

  void setOutputTag(std::string tag) { m_outputTag = tag; }
  std::string getOutputTag() const { return m_outputTag; }
  void setOutputDir(std::string dir) { m_outputDir = dir; }
  std::string getOutputDir() const { return m_outputDir; }

#if Acts_VERSION_MAJOR >= 37
  using Color = Acts::Color;
#else
  using Color = Acts::ColorRGB;
#endif
  void setContainerView(std::array<int, 3> c) { m_containerView.color = Color(c); }
  const Acts::ViewConfig& getContainerView() const { return m_containerView; }
  void setVolumeView(std::array<int, 3> c) { m_volumeView.color = Color(c); }
  const Acts::ViewConfig& getVolumeView() const { return m_volumeView; }
  void setSensitiveView(std::array<int, 3> c) { m_sensitiveView.color = Color(c); }
  const Acts::ViewConfig& getSensitiveView() const { return m_sensitiveView; }
  void setPassiveView(std::array<int, 3> c) { m_passiveView.color = Color(c); }
  const Acts::ViewConfig& getPassiveView() const { return m_passiveView; }
  void setGridView(std::array<int, 3> c) { m_gridView.color = Color(c); }
  const Acts::ViewConfig& getGridView() const { return m_gridView; }
};
