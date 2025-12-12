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
#include <DD4hep/Handle.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/Segmentation.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/CartesianGridUV.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JException.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
// Access "algorithms:GeoSvc"
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <cmath>
#include <exception>
#include <initializer_list>
#include <unordered_map>
#include <utility>

#include "ActsGeometryProvider.h"

namespace eicrecon {

void TrackerMeasurementFromHits::init() {
  // ***** B0Tracker
  m_detid_b0tracker = m_dd4hepGeo->constant<unsigned long>("B0Tracker_Station_1_ID");
  // ***** OuterMPGDBarrel
  std::string readout = "OuterMPGDBarrelHits";
  //  If "CartesianGridUV" segmentation, we need to rotate the cov. matrix.
  // Particularly when 2D-strip readout, with large uncertainty along strips.
  // i) Determine whether "CartesianGridUV", possibly embedded in a
  //   MultiSegmentation.
  // ii) If indeed:
  //   - set "m_detid_OuterMPGD" to single out corresponding hits,
  //   - AND set "m_outermpgd_UVsegmentation_mode", to be also required so as
  //    to avoid accidentally hitting a coincidence for malformed cellID with
  //    systemID = 0.
  const dd4hep::Detector* detector = algorithms::GeoSvc::instance().detector();
  dd4hep::Segmentation seg;
  m_outermpgd_UVsegmentation_mode = true;
  try {
    seg = detector->readout(readout).segmentation();
  } catch (const std::runtime_error&) {
    debug(R"(Failed to load Segmentation for "{}" readout.)", readout);
    m_outermpgd_UVsegmentation_mode = false;
  }
  if (m_outermpgd_UVsegmentation_mode) {
    debug(R"(Parsing Segmentation for "{}" readout: is it UV?)", readout);
    // Segmentation
    using Segmentation               = dd4hep::DDSegmentation::Segmentation;
    const Segmentation* segmentation = seg->segmentation;
    if (segmentation->type() == "MultiSegmentation") {
      // MultiSegmentation
      // - Parse all subSegmentations
      // - Require consistency: either all are UV, w/ same gridAngle, or none.
      //  We could invent a sophisticated scheme where cov. matrix would be
      //  rotated by a subSegmentation-dependent angle, possibly null. But for
      //  the time being, let's keep things simple: throw an exception upon any
      //  inconsistency.
      using MultiSegmentation    = dd4hep::DDSegmentation::MultiSegmentation;
      const auto* multiSeg       = dynamic_cast<const MultiSegmentation*>(segmentation);
      unsigned int subSegPattern = 0;
      for (const auto entry : multiSeg->subSegmentations()) {
        const Segmentation* subSegmentation = entry.segmentation;
        if (subSegmentation->type() == "CartesianGridUV") {
          const dd4hep::DDSegmentation::CartesianGridUV* gridUV =
              dynamic_cast<const dd4hep::DDSegmentation::CartesianGridUV*>(subSegmentation);
          double gridAngle = gridUV->gridAngle();
          if ((subSegPattern & 0x1) && fabs(gridAngle - m_gridAngle) > 1e-6) {
            critical(R"(Inconsistent Segmentation for "{}" readout: UV gridAngle not unique.)",
                     readout);
            throw std::runtime_error("Inconsistent MultiSegmentation");
          }
          subSegPattern |= 0x1;
          m_gridAngle = gridAngle;
        } else {
          subSegPattern |= 0x2;
        }
      }
      if (subSegPattern == 0x3) {
        critical(R"(Inconsistent Segmentation for "{}" readout: only partially UV.)", readout);
        throw std::runtime_error("Inconsistent MultiSegmentation");
      } else if (subSegPattern == 0x2) {
        m_outermpgd_UVsegmentation_mode = false;
      }
    } else {
      if (segmentation->type() != "CartesianGridUV") {
        m_outermpgd_UVsegmentation_mode = false;
	const dd4hep::DDSegmentation::CartesianGridUV* gridUV =
	  dynamic_cast<const dd4hep::DDSegmentation::CartesianGridUV*>(segmentation);
	m_gridAngle = gridUV->gridAngle();
      }
    }
    if (m_outermpgd_UVsegmentation_mode) {
      m_detid_OuterMPGD = m_dd4hepGeo->constant<unsigned long>("TrackerBarrel_5_ID");
    }
  }
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

    auto hit_det = hit.getCellID() & 0xFF;

    Acts::SquareMatrix2 cov = Acts::SquareMatrix2::Zero();
    cov(0, 0)               = hit.getPositionError().xx * mm_acts * mm_acts; // note mm = 1 (Acts)
    cov(1, 1)               = hit.getPositionError().yy * mm_acts * mm_acts;
    cov(0, 1) = cov(1, 0) = 0.0;
    if (m_outermpgd_UVsegmentation_mode && hit_det == m_detid_OuterMPGD) {
      // Note: I(Y.B.) don't how to initialize an "Acts::RotationMatrix2" w/
      // an argument angle. The following turns out to fail:
      // const Acts::RotationMatrix2 rot2(m_gridAngle);
      const double sA = sin(m_gridAngle), cA = cos(m_gridAngle);
      const Acts::RotationMatrix2 rot2{{cA, sA}, {-sA, cA}};
      const Acts::RotationMatrix2 inv2{{cA, -sA}, {sA, cA}};
      cov = rot2 * cov * inv2;
    }

    const auto* vol_ctx = m_converter->findContext(hit.getCellID());
    auto vol_id         = vol_ctx->identifier;

    // trace("Hit preparation information: {}", hit_index);
    trace("   System id: {}, Cell id: {}", hit.getCellID() & 0xFF, hit.getCellID());
    trace("   cov matrix:      {:>12.2e} {:>12.2e}", cov(0, 0), cov(0, 1));
    trace("                    {:>12.2e} {:>12.2e}", cov(1, 0), cov(1, 1));
    trace("   surfaceMap size: {}", surfaceMap.size());

    const auto is = surfaceMap.find(vol_id);
    if (is == surfaceMap.end()) {
      warning(" WARNING: vol_id ({})  not found in m_surfaces.", vol_id);
      continue;
    }
    const Acts::Surface* surface = is->second;
    // variable surf_center not used anywhere;

    const auto& hit_pos = hit.getPosition(); // 3d position

    Acts::Vector2 pos;
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
