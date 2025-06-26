// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#include "LGADHitAssociation.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <TGeoMatrix.h>
#include <algorithms/geo.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <gsl/pointers>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/reco/LGADHitAssociationConfig.h"

namespace eicrecon {

void LGADHitAssociation::init() {
  auto detector = algorithms::GeoSvc::instance().detector();
  auto seg      = detector->readout(m_cfg.readout).segmentation();
  m_converter   = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_decoder     = seg.decoder();
}

dd4hep::rec::CellID LGADHitAssociation::getSensorInfos(const dd4hep::rec::CellID& id) const {
  // If we fix all subsensor key values to zero, what remains will be the sensor information only
  auto id_return = id;
  for (const auto& key : m_cfg.subsensor_keys)
    m_decoder->set(id_return, key, 0);
  return id_return;
}

void LGADHitAssociation::process(const LGADHitAssociation::Input& input,
                                        const LGADHitAssociation::Output& output) const {
  using dd4hep::mm;
  const auto [cal_hits, raw_hits] = input;
  auto [asso_hits]                = output;

  // group raw hit by cell ID
  std::unordered_map<dd4hep::rec::CellID, std::vector<edm4eic::MutableTrackerHit>> cellHitMap;

  for (const auto& hit : *cal_hits) {
      auto asso_hit = asso_hits->create(hit.getCellID(),
		                        hit.getPosition(),
					hit.getPositionError(),
					hit.getTime(),
					hit.getTimeError(),
					hit.getEdep(),
					hit.getEdepError());
      cellHitMap[getSensorInfos(asso_hit.getCellID())].push_back(asso_hit);
  }

  // sort the tracker hits by time
  for (auto& [cellID, hits] : cellHitMap)
    std::sort(hits.begin(), hits.end(),
              [](const edm4eic::MutableTrackerHit& a, const edm4eic::MutableTrackerHit& b) {
                return a.getTime() < b.getTime();
              });

  // get the associated raw_hits by the closest time
  for (const auto& raw_hit : *raw_hits) {
    auto time     = raw_hit.getTimeStamp() / 1e3; // ps->ns
    auto sensorID = getSensorInfos(raw_hit.getCellID());
    auto it       = cellHitMap.find(sensorID);
    if (it != cellHitMap.end()) {
      auto& hits = it->second;
      // if t_raw - assoDeltaT < t_hit < t_raw + assoDeltaT, we make the association
      auto itVec = std::lower_bound(
          hits.begin(), hits.end(), time - m_cfg.assoDeltaT,
          [](const edm4eic::MutableTrackerHit& a, double t) { return a.getTime() < t; });
      for (; itVec != hits.end() && itVec->getTime() <= time + m_cfg.assoDeltaT; ++itVec) {
        itVec->setRawHit(raw_hit);
      }
    }
  }
}

} // namespace eicrecon
