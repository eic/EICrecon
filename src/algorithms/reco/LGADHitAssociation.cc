// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#include "LGADHitAssociation.h"

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <algorithms/geo.h>
#include <algorithm>
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
  const auto [cal_hits, sim_hits] = input;
  auto [associations]             = output;

  // group sim hit by cell ID
  std::unordered_map<dd4hep::rec::CellID, std::vector<edm4hep::SimTrackerHit>> cellHitMap;

  for (const auto& hit : *sim_hits)
    cellHitMap[getSensorInfos(hit.getCellID())].push_back(hit);

  // sort the tracker hits by time
  for (auto& [cellID, hits] : cellHitMap)
    std::sort(hits.begin(), hits.end(),
              [](const edm4hep::SimTrackerHit& a, const edm4hep::SimTrackerHit& b) {
                return a.getTime() < b.getTime();
              });

  // get the associated raw_hits by the closest time
  for (const auto& cal_hit : *cal_hits) {
    auto time     = cal_hit.getTime();
    auto sensorID = getSensorInfos(cal_hit.getCellID());
    auto it       = cellHitMap.find(sensorID);
    if (it != cellHitMap.end()) {
      auto& hits = it->second;
      // if t_raw - assoDeltaT < t_hit < t_raw + assoDeltaT, we make the association
      auto itVec = std::lower_bound(
          hits.begin(), hits.end(), time - m_cfg.assoDeltaT,
          [](const edm4hep::SimTrackerHit& a, double t) { return a.getTime() < t; });
      double deltaT          = std::numeric_limits<double>::max();
      bool associationIsMade = false;
      edm4eic::MutableMCRecoTrackerHitAssociation hitassoc;
      for (; itVec != hits.end() && itVec->getTime() <= time + m_cfg.assoDeltaT; ++itVec) {
        double newDeltaT = std::abs(itVec->getTime() - time);
        if (newDeltaT < deltaT) {
          deltaT = newDeltaT;
          // make association
          if (!associationIsMade) {
            hitassoc          = associations->create();
            associationIsMade = true;
          }
          hitassoc.setWeight(1.0);
          hitassoc.setSimHit(*itVec);
          hitassoc.setRawHit(cal_hit.getRawHit());
        }
      }
    }
  }
}

} // namespace eicrecon
