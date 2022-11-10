// Original license from Gaudi algorithm:
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov
// TODO refactor header when license is clear

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

#include <utility>


void eicrecon::TrackerSourceLinker::init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellid_converter,
                                         std::shared_ptr<const ActsGeometryProvider> acts_context,
                                         std::shared_ptr<spdlog::logger> logger) {
    m_cellid_converter = std::move(cellid_converter);
    m_log = std::move(logger);
    m_acts_context = std::move(acts_context);
}


eicrecon::TrackerSourceLinkerResult *eicrecon::TrackerSourceLinker::produce(std::vector<const edm4eic::TrackerHit*> trk_hits) {
    constexpr double mm_acts = Acts::UnitConstants::mm;
    constexpr double mm_conv = mm_acts / dd4hep::mm; // = 1/0.1

    // input collection
    auto hits = trk_hits;

    // Create output collections
    std::list<Jug::IndexSourceLink> linkStorage;
    auto sourceLinks = std::vector<std::shared_ptr<Jug::IndexSourceLink>>();
    auto measurements = std::make_shared<Jug::MeasurementContainer>();

    m_log->debug("Hits size: {}  measurements->size: {}", trk_hits.size(), measurements->size());

    int hit_index = 0;
    for (auto hit: trk_hits) {

        Acts::SymMatrix2 cov = Acts::SymMatrix2::Zero();
        cov(0, 0) = hit->getPositionError().xx * mm_acts * mm_acts; // note mm = 1 (Acts)
        cov(1, 1) = hit->getPositionError().yy * mm_acts * mm_acts;


        const auto* vol_ctx = m_cellid_converter->findContext(hit->getCellID());
        auto vol_id = vol_ctx->identifier;

        auto surfaceMap = m_acts_context->surfaceMap();

        m_log->trace("Hit preparation information: {}", hit_index);
        m_log->trace("   cov matrix:      {}", cov);
        m_log->trace("   surfaceMap size: {}", surfaceMap.size());

        const auto is = surfaceMap.find(vol_id);
        if (is == m_acts_context->surfaceMap().end()) {
            m_log->warn(" WARNING: vol_id ({})  not found in m_surfaces.", vol_id );
            continue;
        }
        const Acts::Surface* surface = is->second;
        // variable surf_center not used anywhere;
        // auto surf_center = surface->center(Acts::GeometryContext());

        auto& hit_pos = hit->getPosition();

        Acts::Vector2 loc = Acts::Vector2::Zero();
        Acts::Vector2 pos;
        try {
            // transform global position into local coordinates
            // geometry context contains nothing here
            pos = surface->globalToLocal(
                    Acts::GeometryContext(),
                    {hit_pos.x, hit_pos.y, hit_pos.z},
                    {0, 0, 0}).value();


            loc[Acts::eBoundLoc0] = pos[0];
            loc[Acts::eBoundLoc1] = pos[1];
        }
        catch(std::exception &ex) {
            m_log->warn("Cant convert globalToLocal for hit: vol_id={} CellID={} x={} y={} z={}",
                        vol_id, hit->getCellID(), hit_pos.x, hit_pos.y, hit_pos.z);
            continue;
        }

        if (m_log->level() <= spdlog::level::trace) {
            auto volman         = m_acts_context->dd4hepDetector()->volumeManager();
            auto alignment      = volman.lookupDetElement(vol_id).nominal();
            auto local_position = (alignment.worldToLocal({hit_pos.x / mm_conv, hit_pos.y / mm_conv, hit_pos.z / mm_conv})) * mm_conv;

            m_log->trace("   hit position     : {:8<.2f} {:8<.2f} {:8<.2f}", hit_pos.x, hit_pos.y, hit_pos.z);
            m_log->trace("   local position   : {:8<.2f} {:8<.2f} {:8<.2f}", local_position.x(), local_position.y(), local_position.z());
            m_log->trace("   surface center   : {}", surface->center(Acts::GeometryContext()).transpose());
            m_log->trace("   acts local center: {}", pos.transpose());
            m_log->trace("   acts loc pos     : {:8<.2f}, {:8<.2f}", loc[Acts::eBoundLoc0], loc[Acts::eBoundLoc1]);
        }


        // Create source links
        auto sourceLink = std::make_shared<Jug::IndexSourceLink>(surface->geometryId(), hit_index);
        sourceLinks.emplace_back(sourceLink);

        auto measurement = Acts::makeMeasurement(*sourceLink, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
        measurements->emplace_back(std::move(measurement));

        hit_index++;
    }
    m_log->debug("All hits processed measurements->size(): {}", measurements->size());

    auto result = new eicrecon::TrackerSourceLinkerResult();

    result->sourceLinks = sourceLinks;
    result->measurements = measurements;

    return result;
}

