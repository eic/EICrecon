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
#include <algorithms/service.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <cmath>
#include <gsl/pointers>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "DD4hep/Detector.h"
#include "LGADChargeSharing.h"
#include "algorithms/digi/LGADChargeSharingConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

void LGADChargeSharing::init() {
  m_detector  = algorithms::GeoSvc::instance().detector();
  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();

  auto seg  = m_detector->readout(m_cfg.readout).segmentation();
  auto type = seg.type();
  // retrieve meaning of cellID bits
  m_decoder = seg.decoder();
  m_idSpec  = m_detector->readout(m_cfg.readout).idSpec();

  // convert cellID to name value pairs for EvaluatorSvc to determine of different cells are neighbors
  std::function hit_pair_to_map = [this](const dd4hep::rec::CellID& id1,
                                         const dd4hep::rec::CellID& id2) {
    std::unordered_map<std::string, double> params;
    for (const auto& p : m_idSpec.fields()) {
      const std::string& name                  = p.first;
      const dd4hep::IDDescriptor::Field* field = p.second;
      params.emplace(name + "_1", field->value(id1));
      params.emplace(name + "_2", field->value(id2));
      trace("{}_1 = {}", name, field->value(id1));
      trace("{}_2 = {}", name, field->value(id2));
    }
    return params;
  };

  auto& serviceSvc = algorithms::ServiceSvc::instance();
  _is_same_sensor  = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")
                        ->compile(m_cfg.same_sensor_condition, hit_pair_to_map);
}

void LGADChargeSharing::process(const LGADChargeSharing::Input& input,
                                const LGADChargeSharing::Output& output) const {
  const auto [simhits] = input;
  auto [sharedHits]    = output;

  for (const auto& hit : *simhits) {
    auto cellID = hit.getCellID();

    std::unordered_set<dd4hep::rec::CellID> dp;
    std::vector<dd4hep::rec::CellID> neighbors;
    this->_findAllNeighborsInSensor(cellID, neighbors, dp);

    double edep       = hit.getEDep();
    double time       = hit.getTime();
    auto momentum     = hit.getMomentum();
    auto truePos      = hit.getPosition();
    auto localPos_hit = this->_global2Local(
        dd4hep::Position(truePos.x * dd4hep::mm, truePos.y * dd4hep::mm, truePos.z * dd4hep::mm));

    for (const auto neighbor : neighbors) {
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
} // LGADChargeSharing:process

void LGADChargeSharing::_findAllNeighborsInSensor(
    dd4hep::rec::CellID hitCell, std::vector<dd4hep::rec::CellID>& answer,
    std::unordered_set<dd4hep::rec::CellID>& dp) const {
  // search all neighbors with DFS
  answer.push_back(hitCell);
  dp.insert(hitCell);

  for (const auto& field : m_cfg.neighbor_fields) {
    // searchDir should either be +1 or -1
    for (int searchDir = -1; searchDir <= 1; searchDir += 2) {
      auto fieldID  = m_decoder->get(hitCell, field);
      auto testCell = hitCell;
      try {
        m_decoder->set(testCell, field, fieldID + searchDir);
      } catch (const std::runtime_error& err) {
        // catch overflow error
        // ignore if invalid position ID
        continue;
      }

      // check if new cellID really exists
      try {
        auto pos = m_converter->position(testCell);
        if (testCell != m_converter->cellID(pos))
          continue;
      } catch (const std::runtime_error& err) {
        // Ignore CellID that is invalid
        continue;
      }

      // only look for cells that have not been searched
      if (dp.find(testCell) == dp.end()) {
        if (_is_same_sensor(hitCell, testCell)) {
          // inside the same sensor
          this->_findAllNeighborsInSensor(testCell, answer, dp);
        }
      }
    }
  }
}

double LGADChargeSharing::_integralGaus(double mean, double sd, double low_lim,
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

dd4hep::Position LGADChargeSharing::_cell2LocalPosition(const dd4hep::rec::CellID& cell) const {
  auto position = m_converter->position(cell); // global position
  return this->_global2Local(position);
}

dd4hep::Position LGADChargeSharing::_global2Local(const dd4hep::Position& pos) const {
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
