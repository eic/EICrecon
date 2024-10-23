// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang, Prithwish Tribedy
//
// Spread energy desposition from one strip to neighboring strips within sensor boundaries

// Author: Chun Yuen Tsang
// Date: 10/22/2024

#include "DD4hep/Detector.h"
#include "DDRec/Surface.h"
#include "TF1.h"
#include "TMath.h"
#include <Evaluator/DD4hepUnits.h>
#include <TGraph.h>
#include <bitset>
#include <fmt/format.h>
#include <iostream>
#include <vector>

#include "BTOFChargeSharing.h"
#include "Math/SpecFunc.h"
#include "algorithms/digi/TOFHitDigiConfig.h"
#include <algorithms/geo.h>

// using namespace dd4hep;
// using namespace dd4hep::Geometry;
// using namespace dd4hep::DDRec;
// using namespace eicrecon;
using namespace dd4hep::xml;

namespace eicrecon {

void BTOFChargeSharing::init() {
  m_detector = algorithms::GeoSvc::instance().detector();;
  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();

  auto seg       = m_detector->readout("TOFBarrelHits").segmentation();
  auto type      = seg.type();
  if (type != "CartesianGridXY")
    throw std::runtime_error("Unsupported segmentation type: " + type +
                             ". BarrelTOF must use CartesianGridXY.");
  // retrieve meaning of cellID bits
  m_decoder = seg.decoder();
}

void BTOFChargeSharing::_findAllNeighborsInSensor(
    dd4hep::rec::CellID hitCell, std::shared_ptr<std::vector<dd4hep::rec::CellID>>& ans,
    std::unordered_set<dd4hep::rec::CellID>& dp) const {
  // use MST to find all neighbor within a sensor
  // I can probably write down the formula by hand, but why do things manually when computer do
  // everything for you?
  const std::vector<std::pair<int, int>> searchDirs{{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  ans->push_back(hitCell);
  dp.insert(hitCell);

  auto sensorID = this -> _getSensorID(hitCell);
  auto xID      = m_decoder->get(hitCell, "x");
  auto yID      = m_decoder->get(hitCell, "y");
  for (const auto& dir : searchDirs) {
    auto testCell = hitCell;
    try {
      m_decoder->set(testCell, "x", xID + dir.first);
      m_decoder->set(testCell, "y", yID + dir.second);
    } catch (const std::runtime_error& err) {
      // catch overflow error
      // ignore if invalid position ID
      continue;
    }

    try {
      auto pos = m_converter->position(testCell);
      if (testCell != m_converter->cellID(pos))
        continue;
    } catch (const std::invalid_argument& err) {
      // Ignore CellID that is invalid
      continue;
    }

    // only look for cells that have not been searched
    if (dp.find(testCell) == dp.end()) {
      auto testSensorID = _getSensorID(testCell);
      if (testSensorID == sensorID) {
        // inside the same sensor
        this->_findAllNeighborsInSensor(testCell, ans, dp);
      }
    }
  }
}

const dd4hep::rec::CellID
BTOFChargeSharing::_getSensorID(const dd4hep::rec::CellID& hitCell) const {
  // fix x-y, what you left with are ids that corresponds to sensor info
  // cellID may change when position changes.
  auto sensorID = hitCell; //_converter -> cellID(_converter -> position(hitCell));
  m_decoder->set(sensorID, "x", 0);
  m_decoder->set(sensorID, "y", 0);

  return sensorID;
}

double BTOFChargeSharing::_integralGaus(double mean, double sd, double low_lim, double up_lim) const {
  // return integral Gauss(mean, sd) dx from x = low_lim to x = up_lim
  double up  = -0.5 * ROOT::Math::erf(TMath::Sqrt(2) * (mean - up_lim) / sd);
  double low = -0.5 * ROOT::Math::erf(TMath::Sqrt(2) * (mean - low_lim) / sd);
  return up - low;
}

dd4hep::Position BTOFChargeSharing::_cell2LocalPosition(const dd4hep::rec::CellID& cell) const {
  auto position   = m_converter -> position(cell); // global position
  return this -> _global2Local(position);
}

dd4hep::Position BTOFChargeSharing::_global2Local(const dd4hep::Position& pos) const {
  auto geoManager = m_detector->world().volume()->GetGeoManager();
  auto node       = geoManager->FindNode(pos.x(), pos.y(), pos.z());
  auto currMatrix = geoManager->GetCurrentMatrix();

  double g[3], l[3];
  pos.GetCoordinates(g);
  currMatrix->MasterToLocal(g, l);
  dd4hep::Position position;
  position.SetCoordinates(l);
  return position;
}

void BTOFChargeSharing::process(const BTOFChargeSharing::Input& input,
                                const BTOFChargeSharing::Output& output) const {
  const auto [simhits] = input;
  auto [sharedHits] = output;
  std::shared_ptr<std::vector<dd4hep::rec::CellID>> neighbors;

  for(const auto & hit : *simhits) {
    auto cellID = hit.getCellID();
    /*if(m_useCache) {
      auto it = m_cache.find(cellID);
      if(it != m_cache.end())
        neighbors = it -> second;
    }*/

    if(!neighbors){
      std::unordered_set<dd4hep::rec::CellID> dp;
      neighbors = std::make_shared<std::vector<dd4hep::rec::CellID>>();
      this -> _findAllNeighborsInSensor(cellID, neighbors, dp);

      // fill cache
      /*if(m_useCache)
         for(const auto cell : *neighbors)
           m_cache[cell] = neighbors;*/
    }

    double edep = hit.getEDep();
    double time = hit.getTime();
    auto momentum = hit.getMomentum();
    auto truePos = hit.getPosition();
    auto localPos_hit = this -> _global2Local(dd4hep::Position(truePos.x / 10., truePos.y / 10., truePos.z / 10.));

    for(const auto neighbor : *neighbors) {
       // integrate over neighbor area to get total energy deposition
       auto localPos_neighbor = this -> _cell2LocalPosition(neighbor);
       auto cellDimension = m_converter -> cellDimensions(neighbor);

       double edep_cell = edep *
                      _integralGaus(localPos_hit.x(), m_cfg.sigma_sharingx,
                                    localPos_neighbor.x() - 0.5 * cellDimension[0],
                                    localPos_neighbor.x() + 0.5 * cellDimension[0]) *
                      _integralGaus(localPos_hit.y(), m_cfg.sigma_sharingy,
                                    localPos_neighbor.y() - 0.5 * cellDimension[1],
                                    localPos_neighbor.y() + 0.5 * cellDimension[1]);

       if(edep_cell > 0) {
         auto globalPos = m_converter -> position(neighbor);
         auto hit =  sharedHits->create();
         hit.setCellID(neighbor);
         hit.setEDep(edep_cell);
         hit.setTime(time);
         hit.setPosition({globalPos.x(), globalPos.y(), globalPos.z()});
         hit.setMomentum({momentum.x, momentum.y, momentum.z});
       }
    }
  }
} // BTOFChargeSharing:process
} // namespace eicrecon
