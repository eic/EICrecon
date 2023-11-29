// Original header license: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Geometry/detail/DefaultDetectorElementBase.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp>
#include <Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp>
#include <Acts/Plugins/Json/JsonMaterialDecorator.hpp>
#include <Acts/Plugins/Json/MaterialMapJsonConverter.hpp>
#include <Acts/Surfaces/PlanarBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Surfaces/SurfaceArray.hpp>
#include <Acts/Surfaces/SurfaceBounds.hpp>
#include <Acts/Utilities/BinningType.hpp>
#include <Acts/Utilities/Result.hpp>
#include <DD4hep/DetElement.h>
#include <DD4hep/VolumeManager.h>
#include <JANA/JException.h>
#include <TGeoManager.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <stddef.h>
#include <exception>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include "ActsGeometryProvider.h"
#include "extensions/spdlog/SpdlogToActs.h"

// Formatter for Eigen matrices
#if FMT_VERSION >= 90000
#include <Eigen/Core>

template <typename T>
struct fmt::formatter<
    T,
    std::enable_if_t<
        std::is_base_of_v<Eigen::MatrixBase<T>, T>,
        char
    >
> : fmt::ostream_formatter {};
#endif // FMT_VERSION >= 90000

void draw_surfaces(std::shared_ptr<const Acts::TrackingGeometry> trk_geo, const Acts::GeometryContext geo_ctx,
                   const std::string &fname) {
    using namespace Acts;
    std::vector<const Surface *> surfaces;

    trk_geo->visitSurfaces([&](const Acts::Surface *surface) {
        // for now we just require a valid surface
        if (surface == nullptr) {
            std::cout << " Not a surface \n";
            return;
        }
        surfaces.push_back(surface);
    });
    std::ofstream os;
    os.open(fname);
    os << std::fixed << std::setprecision(6);
    size_t nVtx = 0;
    for (const auto &srfx: surfaces) {
        const auto *srf = dynamic_cast<const PlaneSurface *>(srfx);
        const auto *bounds = dynamic_cast<const PlanarBounds *>(&srf->bounds());
        for (const auto &vtxloc: bounds->vertices()) {
            Vector3 vtx = srf->transform(geo_ctx) * Vector3(vtxloc.x(), vtxloc.y(), 0);
            os << "v " << vtx.x() << " " << vtx.y() << " " << vtx.z() << "\n";
        }
        // connect them
        os << "f";
        for (size_t i = 1; i <= bounds->vertices().size(); ++i) {
            os << " " << nVtx + i;
        }
        os << "\n";
        nVtx += bounds->vertices().size();
    }
    os.close();
}


void ActsGeometryProvider::initialize(const dd4hep::Detector* dd4hep_geo,
                                      std::string material_file,
                                      std::shared_ptr<spdlog::logger> log,
                                      std::shared_ptr<spdlog::logger> init_log) {
    // LOGGING
    m_log = log;
    m_init_log = init_log;

    m_init_log->debug("ActsGeometryProvider initializing...");

    m_init_log->debug("Set TGeoManager and acts_init_log_level log levels");
    // Turn off TGeo printouts if appropriate for the msg level
    if (m_log->level() >= (int) spdlog::level::info) {
        TGeoManager::SetVerboseLevel(0);
    }

    // Set ACTS logging level
    auto acts_init_log_level = eicrecon::SpdlogToActsLevel(m_init_log->level());

    m_dd4hepDetector = dd4hep_geo;

    // create a list of all surfaces in the detector:
//  dd4hep::rec::SurfaceManager surfMan( *m_dd4hepDetector ) ;
//
//  m_log->debug(" surface manager ");
//  const auto* const sM = surfMan.map("tracker") ;
//  if (sM != nullptr) {
//      m_log->debug(" surface map  size: {}", sM->size());
//    // setup  dd4hep surface map
//    //for( dd4hep::rec::SurfaceMap::const_iterator it = sM->begin() ; it != sM->end() ; ++it ){
//    for( const auto& [id, s] :   *sM) {
//      //dd4hep::rec::Surface* surf = s ;
//      m_surfaceMap[ id ] = dynamic_cast<dd4hep::rec::Surface*>(s) ;
//        //m_log->debug(" surface : {}", *s );
////      m_detPlaneMap[id] = std::shared_ptr<genfit::DetPlane>(
////          new genfit::DetPlane({s->origin().x(), s->origin().y(), s->origin().z()}, {s->u().x(), s->u().y(), s->u().z()},
////                               {s->v().x(), s->v().y(), s->v().z()}));
//    }
//  }

    // Load ACTS materials maps
    std::shared_ptr<const Acts::IMaterialDecorator> materialDeco{nullptr};
    if (!material_file.empty()) {
        m_init_log->info("loading materials map from file: '{}'", material_file);
        // Set up the converter first
        Acts::MaterialMapJsonConverter::Config jsonGeoConvConfig;
        // Set up the json-based decorator
        materialDeco = std::make_shared<const Acts::JsonMaterialDecorator>(jsonGeoConvConfig, material_file, acts_init_log_level);
    }

    // Geometry identifier hook to write detector ID to extra field
    class ConvertDD4hepDetectorGeometryIdentifierHook: public Acts::GeometryIdentifierHook {
      Acts::GeometryIdentifier decorateIdentifier(
          Acts::GeometryIdentifier identifier, const Acts::Surface& surface) const {
        const auto* dd4hep_det_element =
            dynamic_cast<const Acts::DD4hepDetectorElement*>(surface.associatedDetectorElement());
        if (dd4hep_det_element == nullptr) {
            return identifier;
        }
        // set 8-bit extra field to 8-bit DD4hep detector ID
        return identifier.setExtra(0xff & dd4hep_det_element->identifier());
      };
    };
    auto geometryIdHook = std::make_shared<ConvertDD4hepDetectorGeometryIdentifierHook>();

    // Convert DD4hep geometry to ACTS
    m_init_log->info("Converting DD4Hep geometry to ACTS...");
    auto logger = eicrecon::getSpdlogLogger("CONV", m_log);
    Acts::BinningType bTypePhi = Acts::equidistant;
    Acts::BinningType bTypeR = Acts::equidistant;
    Acts::BinningType bTypeZ = Acts::equidistant;
    double layerEnvelopeR = Acts::UnitConstants::mm;
    double layerEnvelopeZ = Acts::UnitConstants::mm;
    double defaultLayerThickness = Acts::UnitConstants::fm;
    using Acts::sortDetElementsByID;

    try {
        m_trackingGeo = Acts::convertDD4hepDetector(
                m_dd4hepDetector->world(),
                *logger,
                bTypePhi,
                bTypeR,
                bTypeZ,
                layerEnvelopeR,
                layerEnvelopeZ,
                defaultLayerThickness,
                sortDetElementsByID,
                m_trackingGeoCtx,
                materialDeco,
                geometryIdHook);
    }
    catch(std::exception &ex) {
        m_init_log->error("Error during DD4Hep -> ACTS geometry conversion: {}", ex.what());
        m_init_log->info ("Set parameter acts::InitLogLevel=trace to see conversion info and possibly identify failing geometry");
        throw JException(ex.what());
    }

    m_init_log->info("DD4Hep geometry converted!");

    // Visit surfaces
    m_init_log->info("Checking surfaces...");
    if (m_trackingGeo) {
        draw_surfaces(m_trackingGeo, m_trackingGeoCtx, "tracking_geometry.obj");

        m_init_log->debug("visiting all the surfaces  ");
        m_trackingGeo->visitSurfaces([this](const Acts::Surface *surface) {
            // for now we just require a valid surface
            if (surface == nullptr) {
                m_init_log->info("no surface??? ");
                return;
            }
            const auto *det_element =
                    dynamic_cast<const Acts::DD4hepDetectorElement *>(surface->associatedDetectorElement());

            if (det_element == nullptr) {
                m_init_log->error("invalid det_element!!! det_element == nullptr ");
                return;
            }

            // more verbose output is lower enum value
            m_init_log->debug(" det_element->identifier() = {} ", det_element->identifier());
            auto volman = m_dd4hepDetector->volumeManager();
            auto *vol_ctx = volman.lookupContext(det_element->identifier());
            auto vol_id = vol_ctx->identifier;

            if (m_init_log->level() <= spdlog::level::debug) {
                auto de = vol_ctx->element;
                m_init_log->debug("  de.path          = {}", de.path());
                m_init_log->debug("  de.placementPath = {}", de.placementPath());
            }

            this->m_surfaces.insert_or_assign(vol_id, surface);
        });
    }
    else {
        m_init_log->error("m_trackingGeo==null why am I still alive???");
    }

    // Load ACTS magnetic field
    m_init_log->info("Loading magnetic field...");
    m_magneticField = std::make_shared<const eicrecon::BField::DD4hepBField>(m_dd4hepDetector);
    Acts::MagneticFieldContext m_fieldctx{eicrecon::BField::BFieldVariant(m_magneticField)};
    auto bCache = m_magneticField->makeCache(m_fieldctx);
    for (int z: {0, 500, 1000, 1500, 2000, 3000, 4000}) {
        auto b = m_magneticField->getField({0.0, 0.0, double(z)}, bCache).value();
        m_init_log->debug("B(z = {:>5} [mm]) = {} T", z, b.transpose() / Acts::UnitConstants::T);
    }

    m_init_log->info("ActsGeometryProvider initialization complete");
}
