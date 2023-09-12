// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov
//
//  ActsGeometryProvider.h
//
//
//  Created by Julia Hrdinka on 30/03/15.
//
//
#pragma once

// ACTS
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Utilities/Logger.hpp>

// DD4Hep
#include <DD4hep/DD4hepUnits.h>

#include "DD4hepBField.h"

#include <spdlog/spdlog.h>

// Forward declarations
namespace Acts {
    class IMaterialDecorator;
    class Surface;
    class TrackingGeometry;
}

namespace dd4hep {
    class Detector;
}
namespace dd4hep::rec {
    class CellIDPositionConverter;
}

/** Draw the surfaces and save to obj file.
 *  This is useful for debugging the ACTS geometry. The obj file can
 *  be loaded into various tools, such as FreeCAD, for inspection.
 */
void draw_surfaces(std::shared_ptr<const Acts::TrackingGeometry> trk_geo, const std::string &fname);

class ActsGeometryProvider {
public:
    ActsGeometryProvider() {}
    using VolumeSurfaceMap = std::unordered_map<uint64_t, const Acts::Surface *>;

    virtual void initialize(const dd4hep::Detector* dd4hep_geo,
                            std::string material_file,
                            std::shared_ptr<spdlog::logger> log,
                            std::shared_ptr<spdlog::logger> init_log) final;

    /** Gets the ACTS tracking geometry.
     */
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const { return m_trackingGeo;}

    std::shared_ptr<const Acts::MagneticFieldProvider> getFieldProvider() const  { return m_magneticField; }

    const VolumeSurfaceMap &surfaceMap() const  { return m_surfaces; }


    const Acts::GeometryContext& getActsGeometryContext() const {return m_trackingGeoCtx;}

    ///  ACTS general logger that is used for running ACTS
    std::shared_ptr<spdlog::logger> getActsRelatedLogger() const { return m_log; }

    /// Logger that is used for geometry initialization
    /// By default its level the same as ACTS general logger (m_log)
    /// But it might be customized to solely printout geometry information
    std::shared_ptr<spdlog::logger> getActsInitRelatedLogger()  const { return m_init_log; }

private:

    /** DD4hep detector interface class.
     * This is the main dd4hep detector handle.
     * <a href="https://dd4hep.web.cern.ch/dd4hep/reference/classdd4hep_1_1Detector.html">See DD4hep Detector documentation</a>
     */
    const dd4hep::Detector *m_detector = nullptr;

    /// ACTS Logging Level
    Acts::Logging::Level acts_log_level = Acts::Logging::INFO;

    /// ACTS Tracking Geometry Context
    Acts::GeometryContext m_trackingGeoCtx;

    /// ACTS Tracking Geometry
    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeo{nullptr};

    /// ACTS surface lookup container for hit surfaces that generate smeared hits
    VolumeSurfaceMap m_surfaces;

    /** DD4hep CellID tool.
     *  Use to lookup geometry information for a hit with cellid number (int64_t).
     *  <a href="https://dd4hep.web.cern.ch/dd4hep/reference/classdd4hep_1_1rec_1_1CellIDPositionConverter.html">See DD4hep CellIDPositionConverter documentation</a>
     */
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

    /// Acts magnetic field
    std::shared_ptr<const eicrecon::BField::DD4hepBField> m_magneticField = nullptr;

    ///  ACTS general logger that is used for running ACTS
    std::shared_ptr<spdlog::logger> m_log;

    /// Logger that is used for geometry initialization
    /// By default its level the same as ACTS general logger (m_log)
    /// But it might be customized to solely printout geometry information
    std::shared_ptr<spdlog::logger> m_init_log;

};
