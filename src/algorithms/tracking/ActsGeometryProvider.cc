// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include <Acts/Geometry/DetectorElementBase.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp>
#include <Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp>
#include <Acts/Plugins/Json/JsonMaterialDecorator.hpp>
#include <Acts/Plugins/Json/MaterialMapJsonConverter.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/BinningType.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/PlyVisualization3D.hpp>
#include <DD4hep/DetElement.h>
#include <DD4hep/VolumeManager.h>
#include <JANA/JException.h>
#include <TGeoManager.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <exception>
#include <initializer_list>
#include <type_traits>

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
        // Write tracking geometry to collection of obj or ply files
        const Acts::TrackingVolume* world = m_trackingGeo->highestTrackingVolume();
        if (m_objWriteIt) {
          m_init_log->info("Writing obj files to {}...", m_outputDir);
          Acts::ObjVisualization3D objVis;
          Acts::GeometryView3D::drawTrackingVolume(
            objVis, *world, m_trackingGeoCtx,
            m_containerView, m_volumeView, m_passiveView, m_sensitiveView, m_gridView,
            m_objWriteIt, m_outputTag, m_outputDir
          );
        }
        if (m_plyWriteIt) {
          m_init_log->info("Writing ply files to {}...", m_outputDir);
          Acts::PlyVisualization3D plyVis;
          Acts::GeometryView3D::drawTrackingVolume(
            plyVis, *world, m_trackingGeoCtx,
            m_containerView, m_volumeView, m_passiveView, m_sensitiveView, m_gridView,
            m_plyWriteIt, m_outputTag, m_outputDir
          );
        }

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
