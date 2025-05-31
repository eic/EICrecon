// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Shujie Li, Wouter Deconinck

#include "TrackerMeasurementFromHits.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/VolumeManager.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithms/logger.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <exception>
#include <unordered_map>
#include <utility>

#include "ActsGeometryProvider.h"

namespace eicrecon {

void TrackerMeasurementFromHits::init() {
  m_detid_b0tracker = m_dd4hepGeo->constant<unsigned long>("B0Tracker_Station_1_ID");
}

void TrackerMeasurementFromHits::process(const Input& input, const Output& output) const {
  const auto [trk_hits] = input;
  auto [meas2Ds]        = output;

  constexpr double mm_acts = Acts::UnitConstants::mm;
  constexpr double mm_conv = mm_acts / dd4hep::mm; // = 1/0.1

  // output collections
  auto const& surfaceMap = m_acts_context->surfaceMap();

  // To do: add clustering to allow forming one measurement from several hits.
  // For now, one hit = one measurement.
  for (const auto& hit : *trk_hits) {

    Acts::SquareMatrix2 cov = Acts::SquareMatrix2::Zero();
    cov(0, 0)               = hit.getPositionError().xx * mm_acts * mm_acts; // note mm = 1 (Acts)
    cov(1, 1)               = hit.getPositionError().yy * mm_acts * mm_acts;
    cov(0, 1) = cov(1, 0) = 0.0;

    const auto* vol_ctx = m_converter->findContext(hit.getCellID());
    auto vol_id         = vol_ctx->identifier;

    // trace("Hit preparation information: {}", hit_index);
    trace("   System id: {}, Cell id: {}", hit.getCellID() & 0xFF, hit.getCellID());
    trace("   cov matrix:      {:>12.2e} {:>12.2e}", cov(0, 0), cov(0, 1));
    trace("                    {:>12.2e} {:>12.2e}", cov(1, 0), cov(1, 1));
    trace("   surfaceMap size: {}", surfaceMap.size());

    const auto is = surfaceMap.find(vol_id);
    if (is == surfaceMap.end()) {
      warning(" WARNING: vol_id ({:#x})  not found in m_surfaces.", vol_id);
      continue;
    }
    const Acts::Surface* surface = is->second;
    // variable surf_center not used anywhere;

    const auto& hit_pos = hit.getPosition(); // 3d position

    Acts::Vector2 pos;
    auto hit_det = hit.getCellID() & 0xFF;
    auto onSurfaceTolerance =
        0.1 *
        Acts::UnitConstants::um; // By default, ACTS uses 0.1 micron as the on surface tolerance
    if (hit_det == m_detid_b0tracker) {
      onSurfaceTolerance =
          1 *
          Acts::UnitConstants::
              um; // FIXME Ugly hack for testing B0. Should be a way to increase this tolerance in geometry.
    }

    try {
      // transform global position into local coordinates
      // geometry context contains nothing here
      pos = surface
                ->globalToLocal(Acts::GeometryContext(), {hit_pos.x, hit_pos.y, hit_pos.z},
                                {0, 0, 0}, onSurfaceTolerance)
                .value();

    } catch (std::exception& ex) {
      warning("Can't convert globalToLocal for hit: vol_id={} det_id={} CellID={} x={} y={} z={}",
              vol_id, hit.getCellID() & 0xFF, hit.getCellID(), hit_pos.x, hit_pos.y, hit_pos.z);
      continue;
    }

    if (level() <= algorithms::LogLevel::kTrace) {

      Acts::Vector2 loc     = Acts::Vector2::Zero();
      loc[Acts::eBoundLoc0] = pos[0];
      loc[Acts::eBoundLoc1] = pos[1];

      auto volman         = m_acts_context->dd4hepDetector()->volumeManager();
      auto alignment      = volman.lookupDetElement(vol_id).nominal();
      auto local_position = (alignment.worldToLocal(
                                {hit_pos.x / mm_conv, hit_pos.y / mm_conv, hit_pos.z / mm_conv})) *
                            mm_conv;
      double surf_center_x = surface->center(Acts::GeometryContext()).transpose()[0];
      double surf_center_y = surface->center(Acts::GeometryContext()).transpose()[1];
      double surf_center_z = surface->center(Acts::GeometryContext()).transpose()[2];
      trace("   hit position     : {:>10.2f} {:>10.2f} {:>10.2f}", hit_pos.x, hit_pos.y, hit_pos.z);
      trace("   local position   : {:>10.2f} {:>10.2f} {:>10.2f}", local_position.x(),
            local_position.y(), local_position.z());
      trace("   surface center   : {:>10.2f} {:>10.2f} {:>10.2f}", surf_center_x, surf_center_y,
            surf_center_z);
      trace("   acts local center: {:>10.2f} {:>10.2f}", pos.transpose()[0], pos.transpose()[1]);
      trace("   acts loc pos     : {:>10.2f} {:>10.2f}", loc[Acts::eBoundLoc0],
            loc[Acts::eBoundLoc1]);
    }

    auto meas2D = meas2Ds->create();
    meas2D.setSurface(surface->geometryId().value()); // Surface for bound coordinates (geometryID)
    meas2D.setLoc(
        {static_cast<float>(pos[0]), static_cast<float>(pos[1])}); // 2D location on surface
    meas2D.setTime(hit.getTime());                                 // Measurement time
    // fixme: no off-diagonal terms. cov(0,1) = cov(1,0)??
    meas2D.setCovariance({cov(0, 0), cov(1, 1), hit.getTimeError() * hit.getTimeError(),
                          cov(0, 1)}); // Covariance on location and time
    meas2D.addToWeights(1.0);          // Weight for each of the hits, mirrors hits array
    meas2D.addToHits(hit);
  }

  debug("All hits processed. Hits size: {}  measurements->size: {}", trk_hits->size(),
        meas2Ds->size());
}

} // namespace eicrecon
