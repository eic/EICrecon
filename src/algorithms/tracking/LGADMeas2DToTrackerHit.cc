// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#include "LGADMeas2DToTrackerHit.h"

#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <algorithms/geo.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector3f.h>
#include <cstddef>
#include <limits>

#include "ActsGeometryProvider.h"
#include "algorithms/interfaces/ActsSvc.h"

namespace eicrecon {

void LGADMeas2DToTrackerHit::init() {
  m_converter    = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_acts_context = algorithms::ActsSvc::instance().acts_geometry_provider();
}

void LGADMeas2DToTrackerHit::process(const LGADMeas2DToTrackerHit::Input& input,
                                     const LGADMeas2DToTrackerHit::Output& output) const {
  const auto [meas2Ds] = input;
  auto [rec_hits]      = output;

  const auto& surfaceMap = m_acts_context->surfaceMap();

  for (const auto& meas2D : *meas2Ds) {
    const auto hits = meas2D.getHits();
    if (hits.empty()) {
      continue;
    }

    // Find the constituent hit with the largest energy deposit; use its cellID/rawHit
    std::size_t idMaxHit = 0;
    auto max_charge      = std::numeric_limits<float>::lowest();
    double tot_charge    = 0;
    auto earliest_time   = std::numeric_limits<float>::max();
    float time_err       = 0;
    for (std::size_t id = 0; id < hits.size(); ++id) {
      const auto& hit = hits[id];
      const auto Edep = hit.getEdep();
      tot_charge += Edep;
      if (Edep > max_charge) {
        max_charge = Edep;
        idMaxHit   = id;
      }
      if (hit.getTime() < earliest_time) {
        earliest_time = hit.getTime();
        time_err      = hit.getTimeError();
      }
    }

    const auto& maxHit               = hits[idMaxHit];
    const dd4hep::rec::CellID cellID = maxHit.getCellID();
    if (!cellID) {
      continue;
    }

    // Look up the Acts surface for this measurement
    const auto* context = m_converter->findContext(cellID);
    auto volID          = context->identifier;
    const auto is       = surfaceMap.find(volID);
    if (is == surfaceMap.end()) {
      error("vol_id ({}) not found in m_surfaces.", volID);
      continue;
    }
    const Acts::Surface* surface = is->second;

    const auto& locPos = meas2D.getLoc();
    auto globalPos     = surface->localToGlobal(m_acts_context->getActsGeometryContext(),
                                                {locPos.a, locPos.b}, {0., 0., 0.});

    auto rec_hit = rec_hits->create(
        cellID,
        edm4hep::Vector3f{static_cast<float>(globalPos.x()), static_cast<float>(globalPos.y()),
                          static_cast<float>(globalPos.z())},
        edm4eic::CovDiag3f{0., 0., 0.}, earliest_time, time_err, static_cast<float>(tot_charge),
        0.0F);

    rec_hit.setRawHit(maxHit.getRawHit());
  }
}

} // namespace eicrecon
