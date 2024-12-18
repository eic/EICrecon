// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang, Prithwish Tribedy
//
// Spread energy deposition from one strip to neighboring strips within sensor boundaries

#include <DD4hep/DetElement.h>
#include <DD4hep/Handle.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Volumes.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <algorithms/geo.h>
#include <cmath>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <gsl/pointers>
#include <stdexcept>
#include <utility>
#include <vector>

#include "BTOFChargeSharing.h"
#include "DD4hep/Detector.h"
#include "algorithms/digi/BTOFChargeSharingConfig.h"

namespace eicrecon {

void BTOFChargeSharing::init() {
  m_detector = algorithms::GeoSvc::instance().detector();
  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();

  auto seg  = m_detector->readout(m_cfg.readout).segmentation();
  auto type = seg.type();
  if (type != "CartesianGridXY")
    throw std::runtime_error("Unsupported segmentation type: " + type +
                             ". BarrelTOF must use CartesianGridXY.");
  // retrieve meaning of cellID bits
  m_decoder = seg.decoder();
}

void BTOFChargeSharing::process(const BTOFChargeSharing::Input& input,
                                const BTOFChargeSharing::Output& output) const {
  const auto [simhits] = input;
  auto [sharedHits]    = output;
  std::shared_ptr<std::vector<dd4hep::rec::CellID>> neighbors;

  for (const auto& hit : *simhits) {
    auto cellID = hit.getCellID();

    if (!neighbors) {
      std::unordered_set<dd4hep::rec::CellID> dp;
      neighbors = std::make_shared<std::vector<dd4hep::rec::CellID>>();
      this->_findAllNeighborsInSensor(cellID, neighbors, dp);
    }

    double edep       = hit.getEDep();
    double time       = hit.getTime();
    auto momentum     = hit.getMomentum();
    auto truePos      = hit.getPosition();
    auto localPos_hit = this->_global2Local(
        dd4hep::Position(truePos.x * dd4hep::mm, truePos.y * dd4hep::mm, truePos.z * dd4hep::mm));

    for (const auto neighbor : *neighbors) {
      // integrate over neighbor area to get total energy deposition
      auto localPos_neighbor = this->_cell2LocalPosition(neighbor);
      auto cellDimension     = m_converter->cellDimensions(neighbor);

      double edep_cell = edep *
                         _integralGaus(localPos_hit.x(), m_cfg.sigma_sharingx,
                                       localPos_neighbor.x() - 0.5 * cellDimension[0],
                                       localPos_neighbor.x() + 0.5 * cellDimension[0]) *
                         _integralGaus(localPos_hit.y(), m_cfg.sigma_sharingy,
                                       localPos_neighbor.y() - 0.5 * cellDimension[1],
                                       localPos_neighbor.y() + 0.5 * cellDimension[1]);

      if (edep_cell > 0) {
        auto globalPos = m_converter->position(neighbor);
        auto hit       = sharedHits->create();
        hit.setCellID(neighbor);
        hit.setEDep(edep_cell);
        hit.setTime(time);
        hit.setPosition({globalPos.x(), globalPos.y(), globalPos.z()});
        hit.setMomentum({momentum.x, momentum.y, momentum.z});
      }
    }
  }
} // BTOFChargeSharing:process

void BTOFChargeSharing::_findAllNeighborsInSensor(
    dd4hep::rec::CellID hitCell, std::shared_ptr<std::vector<dd4hep::rec::CellID>>& answer,
    std::unordered_set<dd4hep::rec::CellID>& dp) const {
  // use MST to find all neighbor within a sensor
  // I can probably write down the formula by hand, but why do things manually when computer do
  // everything for you?
  const std::vector<std::pair<int, int>> searchDirs{{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
  answer->push_back(hitCell);
  dp.insert(hitCell);

  auto sensorID = this->_getSensorID(hitCell);
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
        this->_findAllNeighborsInSensor(testCell, answer, dp);
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

double BTOFChargeSharing::_integralGaus(double mean, double sd, double low_lim,
                                        double up_lim) const {
  // return integral Gauss(mean, sd) dx from x = low_lim to x = up_lim
  // default value is set when sd = 0
  double up  = mean > up_lim ? -0.5 : 0.5;
  double low = mean > low_lim ? -0.5 : 0.5;
  if (sd > 0) {
    up  = -0.5 * std::erf(std::sqrt(2) * (mean - up_lim) / sd);
    low = -0.5 * std::erf(std::sqrt(2) * (mean - low_lim) / sd);
  }
  return up - low;
}

dd4hep::Position BTOFChargeSharing::_cell2LocalPosition(const dd4hep::rec::CellID& cell) const {
  auto position = m_converter->position(cell); // global position
  return this->_global2Local(position);
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

} // namespace eicrecon
