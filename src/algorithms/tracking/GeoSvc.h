// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

//
//  GeoSvc.h
//
//
//  Created by Julia Hrdinka on 30/03/15.
//
//

#ifndef GEOSVC_H
#define GEOSVC_H

// Interface
#include "JugBase/IGeoSvc.h"

// ACTS
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"
#include <Acts/Material/IMaterialDecorator.hpp>

// DD4Hep
#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"
#include "DD4hep/DD4hepUnits.h"

#include "JugBase/BField/DD4hepBField.h"

#include <spdlog/spdlog.h>

/** Draw the surfaces and save to obj file.
 *  This is useful for debugging the ACTS geometry. The obj file can
 *  be loaded into various tools, such as FreeCAD, for inspection.
 */
void draw_surfaces(std::shared_ptr<const Acts::TrackingGeometry> trk_geo, const std::string& fname);

class GeoSvc: public IGeoSvc {
public:
  using VolumeSurfaceMap = std::unordered_map<uint64_t, const Acts::Surface*>;

private:

  /** DD4hep detector interface class.
   * This is the main dd4hep detector handle.
   * <a href="https://dd4hep.web.cern.ch/dd4hep/reference/classdd4hep_1_1Detector.html">See DD4hep Detector documentation</a>
   */
  dd4hep::Detector* m_dd4hepGeo = nullptr;

  /// DD4hep surface map
  std::map< int64_t, dd4hep::rec::Surface* > m_surfaceMap ;

  /// Genfit DetPlane map
  std::map< int64_t, std::shared_ptr<genfit::DetPlane> > m_detPlaneMap ;

  /// ACTS Logging Level
  Acts::Logging::Level m_actsLoggingLevel = Acts::Logging::INFO;

  /// ACTS Tracking Geometry Context
  Acts::GeometryContext m_trackingGeoCtx;

  /// ACTS Tracking Geometry
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeo{nullptr};

  /// ACTS Material Decorator
  std::shared_ptr<const Acts::IMaterialDecorator> m_materialDeco{nullptr};

  /// ACTS surface lookup container for hit surfaces that generate smeared hits
  VolumeSurfaceMap m_surfaces;

  /** DD4hep CellID tool.
   *  Use to lookup geometry information for a hit with cellid number (int64_t).
   *  <a href="https://dd4hep.web.cern.ch/dd4hep/reference/classdd4hep_1_1rec_1_1CellIDPositionConverter.html">See DD4hep CellIDPositionConverter documentation</a>
   */
  std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

  /// Acts magnetic field
  std::shared_ptr<const Jug::BField::DD4hepBField> m_magneticField = nullptr;

//  /// XML-files with the detector description
//  Gaudi::Property<std::vector<std::string>> m_xmlFileNames{
//      this, "detectors", {}, "Detector descriptions XML-files"};
//
//  /// JSON-file with the material map
//  Gaudi::Property<std::string> m_jsonFileName{
//      this, "materials", "", "Material map JSON-file"};
//
//  /// Gaudi logging output
//  MsgStream m_log;

public:




  virtual void initialize(dd4hep::Detector* dd4hepGeo) final;

  virtual void finalize() final;

  /** Build the dd4hep geometry.
   * This function generates the DD4hep geometry.
   */
  void buildDD4HepGeo();

  /** Get the top level DetElement.
   *   DD4hep Geometry
   */
  virtual dd4hep::DetElement getDD4HepGeo();

  /** Get the main dd4hep Detector.
   * Returns the pointer to the main dd4hep detector class.
   * <a href="https://dd4hep.web.cern.ch/dd4hep/reference/classdd4hep_1_1Detector.html">See DD4hep Detector documentation</a>
   */
  virtual dd4hep::Detector* detector();

  /** Gets the ACTS tracking geometry.
   */
  virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const;

  virtual std::shared_ptr<const Acts::MagneticFieldProvider> getFieldProvider() const { return m_magneticField; }

  virtual double centralMagneticField() const
  {
    return m_dd4hepGeo->field().magneticField({0, 0, 0}).z() * (Acts::UnitConstants::T / dd4hep::tesla);
  }

  virtual const VolumeSurfaceMap& surfaceMap() const { return m_surfaces; }

  // Note this hsould return a const& but is just copied for the moment to get around genfit's api
  virtual std::map<int64_t, std::shared_ptr<genfit::DetPlane>> getDetPlaneMap() const { return m_detPlaneMap; }

  virtual std::map< int64_t, dd4hep::rec::Surface* > getDD4hepSurfaceMap() const { return m_surfaceMap ;}


    std::shared_ptr<spdlog::logger> m_log;
};

inline std::shared_ptr<const Acts::TrackingGeometry> GeoSvc::trackingGeometry() const
{
  return m_trackingGeo;
}

#endif // GEOSVC_H
