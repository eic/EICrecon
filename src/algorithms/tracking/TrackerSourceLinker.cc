// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackerSourceLinker.h"
#include "TrackerSourceLinkerResult.h"

#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Volumes.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/Surface.h"
#include "DDRec/SurfaceManager.h"

#include "Acts/Definitions/Common.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>


void eicrecon::TrackerSourceLinker::init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellid_converter,
                               std::shared_ptr<spdlog::logger> logger) {
    m_cellid_converter = cellid_converter;
    m_log = logger;
}


eicrecon::TrackerSourceLinkerResult *eicrecon::TrackerSourceLinker::produce(std::vector<const edm4eic::TrackerHit*> trk_hits) {
    constexpr double mm_acts = Acts::UnitConstants::mm;
    constexpr double mm_conv = mm_acts / dd4hep::mm; // = 1/0.1

    // input collection
    auto hits = trk_hits;
    // Create output collections
    std::list<Jug::IndexSourceLink> linkStorage;
    Jug::IndexSourceLinkContainer sourceLinks;
    Jug::MeasurementContainer measurements;

    sourceLinks.reserve(trk_hits.size());
    measurements.reserve(trk_hits.size());

    m_log->debug("Hits size: {}", trk_hits.size());

    int ihit = 0;
    for (auto ahit: trk_hits) {

        Acts::SymMatrix2 cov = Acts::SymMatrix2::Zero();
        cov(0, 0)            = ahit->getPositionError().xx * mm_acts * mm_acts; // note mm = 1 (Acts)
        cov(1, 1)            = ahit->getPositionError().yy * mm_acts * mm_acts;

        m_log->debug("cov matrix:\n {}", cov);


        const auto* vol_ctx = m_cellid_converter->findContext(ahit->getCellID());
        auto vol_id = vol_ctx->identifier;


        const auto is = m_acts_context->surfaceMap().find(vol_id);
        if (is == m_acts_context->surfaceMap().end()) {
            m_log->warn(" WARNING: vol_id ({})  not found in m_surfaces.", vol_id );
            continue;
        }
        const Acts::Surface* surface = is->second;
        // variable surf_center not used anywhere;
        // auto surf_center = surface->center(Acts::GeometryContext());

        auto& hit_pos = ahit->getPosition();

        // transform global position into local coordinates
        // geometry context contains nothing here
        Acts::Vector2 pos = surface->globalToLocal(
                Acts::GeometryContext(),
                {hit_pos.x, hit_pos.y, hit_pos.z},
                {0, 0, 0}).value();

        Acts::Vector2 loc     = Acts::Vector2::Zero();
        loc[Acts::eBoundLoc0] = pos[0];
        loc[Acts::eBoundLoc1] = pos[1];

        if (m_log->level() <= spdlog::level::debug) {

            auto volman         = m_acts_context->dd4hepDetector()->volumeManager();
            auto alignment      = volman.lookupDetElement(vol_id).nominal();
            auto local_position = (alignment.worldToLocal({hit_pos.x / mm_conv, hit_pos.y / mm_conv, hit_pos.z / mm_conv})) * mm_conv;

            m_log->debug(" hit position     : {} {} {}", hit_pos.x, hit_pos.y, hit_pos.z);
            m_log->debug(" local position   : {} {} {}", local_position.x(), local_position.y(), local_position.z());
            m_log->debug(" surface center   : {}", surface->center(Acts::GeometryContext()).transpose());
            m_log->debug(" acts local center: {}", pos.transpose());
            m_log->debug(" acts loc pos     : {}, {}", loc[Acts::eBoundLoc0], loc[Acts::eBoundLoc1]);
        }

        // the measurement container is unordered and the index under which the
        // measurement will be stored is known before adding it.
        //
        // variable hitIdx not used anywhere
        // Index hitIdx = measurements->size();
        linkStorage.emplace_back(surface->geometryId(), ihit);
        Jug::IndexSourceLink& sourceLink = linkStorage.back();
        auto meas = Acts::makeMeasurement(sourceLink, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);


        // add to output containers. since the input is already geometry-order,
        // new elements in geometry containers can just be appended at the end.
        sourceLinks.emplace_hint(sourceLinks.end(), sourceLink);
        measurements.emplace_back(std::move(meas));

        ihit++;
    }
    auto result = new eicrecon::TrackerSourceLinkerResult;
    result->sourceLinks = sourceLinks;
    result->measurements = measurements;
    return result;
}

