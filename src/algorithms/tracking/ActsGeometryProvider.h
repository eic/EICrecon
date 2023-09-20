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

#include <spdlog/spdlog.h>

// ACTS
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>

// Forward declarations
namespace Acts {
    class MagneticFieldProvider;
    class Surface;
    class TrackingGeometry;
}
namespace dd4hep {
    class Detector;
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

    virtual void initialize(const dd4hep::Detector* detector,
                            std::string material_file,
                            std::shared_ptr<spdlog::logger> log,
                            std::shared_ptr<spdlog::logger> init_log) final;

    /** Gets the ACTS tracking geometry.
     */
    std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const { return m_trackingGeo;}

    std::shared_ptr<const Acts::MagneticFieldProvider> getFieldProvider() const  { return m_magneticField; }

    const VolumeSurfaceMap &surfaceMap() const  { return m_surfaces; }


    const Acts::GeometryContext& getActsGeometryContext() const {return m_trackingGeoCtx;}
    const Acts::MagneticFieldContext& getActsMagneticFieldContext() const {return m_magneticFieldCtx;}

private:

    /// ACTS Tracking Geometry Context
    Acts::GeometryContext m_trackingGeoCtx;
    Acts::MagneticFieldContext m_magneticFieldCtx;

    /// ACTS Tracking Geometry
    std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeo{nullptr};

    /// ACTS surface lookup container for hit surfaces that generate smeared hits
    VolumeSurfaceMap m_surfaces;

    /// Acts magnetic field
    std::shared_ptr<const Acts::MagneticFieldProvider> m_magneticField = nullptr;
};
