// Original header license: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include <fmt/ostream.h>

#include "ActsGeometryProvider.h"

#include <TGeoManager.h>

#include "ActsExamples/Geometry/MaterialWiper.hpp"

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp>
#include <Acts/Plugins/Json/JsonMaterialDecorator.hpp>
#include <Acts/Plugins/Json/MaterialMapJsonConverter.hpp>
#include <Acts/Utilities/Logger.hpp>

#include <DD4hep/Detector.h>

#include "extensions/spdlog/SpdlogToActs.h"
#include "extensions/spdlog/SpdlogFormatters.h"

#include "DD4hepBField.h"

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


void ActsGeometryProvider::initialize(const dd4hep::Detector* detector,
                                      std::string material_file,
                                      std::shared_ptr<spdlog::logger> log,
                                      std::shared_ptr<spdlog::logger> init_log) {

    init_log->debug("ActsGeometryProvider initializing...");

    init_log->debug("Set TGeoManager and acts_init_log_level log levels");
    // Turn off TGeo printouts if appropriate for the msg level
    if (log->level() >= (int) spdlog::level::info) {
        TGeoManager::SetVerboseLevel(0);
    }

    // Set ACTS logging level
    auto acts_init_log_level = eicrecon::SpdlogToActsLevel(init_log->level());

    // Load ACTS materials maps
    std::shared_ptr<const Acts::IMaterialDecorator> materialDeco{nullptr};
    if (!material_file.empty()) {
        init_log->info("loading materials map from file: '{}'", material_file);
        // Set up the converter first
        Acts::MaterialMapJsonConverter::Config jsonGeoConvConfig;
        // Set up the json-based decorator
        materialDeco = std::make_shared<const Acts::JsonMaterialDecorator>(jsonGeoConvConfig, material_file, acts_init_log_level);
    } else {
        init_log->warn("no ACTS materials map has been loaded");
        materialDeco = std::make_shared<const Acts::MaterialWiper>();
    }

    // Convert DD4hep geometry to ACTS
    init_log->info("Converting DD4Hep geometry to ACTS...");
    Acts::BinningType bTypePhi = Acts::equidistant;
    Acts::BinningType bTypeR = Acts::equidistant;
    Acts::BinningType bTypeZ = Acts::equidistant;
    double layerEnvelopeR = Acts::UnitConstants::mm;
    double layerEnvelopeZ = Acts::UnitConstants::mm;
    double defaultLayerThickness = Acts::UnitConstants::fm;
    using Acts::sortDetElementsByID;

    try {
        m_trackingGeo = Acts::convertDD4hepDetector(
                detector->world(),
                acts_init_log_level,
                bTypePhi,
                bTypeR,
                bTypeZ,
                layerEnvelopeR,
                layerEnvelopeZ,
                defaultLayerThickness,
                sortDetElementsByID,
                m_trackingGeoCtx,
                materialDeco);
    }
    catch(std::exception &ex) {
        init_log->error("Error during DD4Hep -> ACTS geometry conversion. See error reason below...");
        init_log->info ("Set parameter acts::InitLogLevel=trace to see conversion info and possibly identify failing geometry");
        throw JException(ex.what());
    }

    init_log->info("DD4Hep geometry converted!");

    // Visit surfaces
    init_log->info("Checking surfaces...");
    if (m_trackingGeo) {
        draw_surfaces(m_trackingGeo, m_trackingGeoCtx, "tracking_geometry.obj");

        init_log->debug("visiting all the surfaces  ");
        m_trackingGeo->visitSurfaces([this,detector,init_log](const Acts::Surface *surface) {
            // for now we just require a valid surface
            if (surface == nullptr) {
                init_log->info("no surface??? ");
                return;
            }
            const auto *det_element =
                    dynamic_cast<const Acts::DD4hepDetectorElement *>(surface->associatedDetectorElement());

            if (det_element == nullptr) {
                init_log->error("invalid det_element!!! det_element == nullptr ");
                return;
            }

            // more verbose output is lower enum value
            init_log->debug(" det_element->identifier() = {} ", det_element->identifier());
            auto volman = detector->volumeManager();
            auto *vol_ctx = volman.lookupContext(det_element->identifier());
            auto vol_id = vol_ctx->identifier;

            if (init_log->level() <= spdlog::level::debug) {
                auto de = vol_ctx->element;
                init_log->debug("  de.path          = {}", de.path());
                init_log->debug("  de.placementPath = {}", de.placementPath());
            }

            this->m_surfaces.insert_or_assign(vol_id, surface);
        });
    }
    else {
        init_log->error("m_trackingGeo==null why am I still alive???");
    }

    // Load ACTS magnetic field
    init_log->info("Loading magnetic field...");
    m_magneticField = std::make_shared<const eicrecon::BField::DD4hepBField>(detector);
    auto bCache = m_magneticField->makeCache(m_magneticFieldCtx);
    for (int z: {0, 500, 1000, 1500, 2000, 3000, 4000}) {
        auto b = m_magneticField->getField({0.0, 0.0, double(z)}, bCache).value();
        init_log->debug("B(z = {:>5} [mm]) = {} T", z, b.transpose());
    }

    init_log->info("ActsGeometryProvider initialization complete");
}
