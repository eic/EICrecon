// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#include "LGADHitClusterAssociation.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <TGeoMatrix.h>
#include <algorithms/geo.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <gsl/pointers>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/reco/LGADHitClusterAssociationConfig.h"

namespace eicrecon {

void LGADHitClusterAssociation::init() {
  auto detector = algorithms::GeoSvc::instance().detector();
  auto seg      = detector->readout(m_cfg.readout).segmentation();
  m_converter   = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_decoder     = seg.decoder();
}

dd4hep::rec::CellID LGADHitClusterAssociation::getSensorInfos(const dd4hep::rec::CellID& id) const {
  // CellID for BarrelTOF is composed of 6 parts
  // system, layer, module, sensor, x, y
  // If we fix x and y to zero, what remains will be the sensor information only
  auto id_return = id;
  m_decoder->set(id_return, "x", 0);
  m_decoder->set(id_return, "y", 0);
  return id_return;
}

edm4hep::Vector3f
LGADHitClusterAssociation::_local2Global(const dd4hep::VolumeManagerContext* context,
                                         const edm4hep::Vector2f& locPos) const {
  auto nodeMatrix = context->element.nominal().worldTransformation();

  double g[3], l[3];
  l[0] = locPos.a * dd4hep::mm;
  l[1] = locPos.b * dd4hep::mm;
  l[2] = 0;
  nodeMatrix.LocalToMaster(l, g);
  return edm4hep::Vector3f{static_cast<float>(g[0] / dd4hep::mm),
                           static_cast<float>(g[1] / dd4hep::mm),
                           static_cast<float>(g[2] / dd4hep::mm)};
}

void LGADHitClusterAssociation::process(const LGADHitClusterAssociation::Input& input,
                                        const LGADHitClusterAssociation::Output& output) const {
  using dd4hep::mm;
  const auto [meas2D_hits, raw_hits] = input;
  auto [asso_hits]                   = output;

  // group raw hit by cell ID
  std::unordered_map<dd4hep::rec::CellID, std::vector<edm4eic::MutableTrackerHit>> cellHitMap;

  for (const auto& meas2D_hit : *meas2D_hits) {
    auto time = meas2D_hit.getTime();

    // sum ADC info
    double tot_charge          = 0;
    dd4hep::rec::CellID cellID = 0;
    double maxCharge           = 0;
    double time_err            = 0;
    double tot_charge_err2     = 0;

    for (const auto& hit : meas2D_hit.getHits()) {
      if (hit.getEdep() > maxCharge) {
        cellID    = hit.getCellID();
        maxCharge = hit.getEdep();
      }
      tot_charge_err2 += hit.getEdepError() * hit.getEdepError();
      tot_charge += hit.getEdep();
    }

    if(cellID) {
      // position info
      auto locPos         = meas2D_hit.getLoc();
      auto locPosErr      = meas2D_hit.getCovariance();
      const auto* context = m_converter->findContext(cellID);
      auto ave_pos        = this->_local2Global(context, locPos);
      auto asso_hit = asso_hits->create(cellID, 
		                        ave_pos, 
		                        edm4eic::CovDiag3f{locPosErr.xx, locPosErr.yy, 0}, 
					time, 
					locPosErr.zz,
                                        tot_charge, 
					std::sqrt(tot_charge_err2));
      cellHitMap[getSensorInfos(asso_hit.getCellID())].push_back(asso_hit);
    }
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
