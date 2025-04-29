// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#include "LGADHitClustering.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Handle.h>
#include <DD4hep/Readout.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <ROOT/RVec.hxx>
#include <TGeoMatrix.h>
#include <algorithms/geo.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector2f.h>
#include <algorithm>
#include <gsl/pointers>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/reco/LGADHitClusteringConfig.h"

namespace eicrecon {

void LGADHitClustering::init() {

  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_detector  = algorithms::GeoSvc::instance().detector();
  m_seg       = m_detector->readout(m_cfg.readout).segmentation();
  auto type   = m_seg.type();
  m_decoder   = m_seg.decoder();
}

dd4hep::rec::CellID LGADHitClustering::getSensorInfos(const dd4hep::rec::CellID& id) const {
  // CellID for BarrelTOF is composed of 6 parts
  // system, layer, module, sensor, x, y
  // If we fix x and y to zero, what remains will be the sensor information only
  auto id_return = id;
  m_decoder->set(id_return, "x", 0);
  m_decoder->set(id_return, "y", 0);
  return id_return;
}

dd4hep::Position LGADHitClustering::_local2Global(const dd4hep::VolumeManagerContext* context,
                                                  const edm4hep::Vector2f& locPos) const {
  auto nodeMatrix = context->element.nominal().worldTransformation();

  double g[3], l[3];
  l[0] = locPos.a * dd4hep::mm;
  l[1] = locPos.b * dd4hep::mm;
  l[2] = 0;
  nodeMatrix.LocalToMaster(l, g);
  dd4hep::Position position;
  position.SetCoordinates(g);
  return position;
}

void LGADHitClustering::process(const LGADHitClustering::Input& input,
                                const LGADHitClustering::Output& output) const {
  using dd4hep::mm;

  const auto [calibrated_hits] = input;
  auto [clusters]              = output;

  // collection of ADC values from all sensors and group them by sensor
  std::unordered_map<dd4hep::rec::CellID, std::vector<edm4eic::TrackerHit>> hitsBySensors;

  for (const auto& calibrated_hit : *calibrated_hits) {

    auto id = calibrated_hit.getCellID();
    // Get position and dimension
    auto pos = m_converter->position(id);
    // Get sensors info
    auto sensorID = this->getSensorInfos(id);
    hitsBySensors[sensorID].emplace_back(calibrated_hit);
  }

  for (const auto& sensor : hitsBySensors) {
    auto cluster = clusters->create();
    // Right now the clustering algorithm a simple average over all hits in a sensors
    // Will be problematic near the edges, but it's just an illustration
    float ave_x = 0, ave_y = 0, ave_z = 0;
    double tot_charge = 0;
    const auto& hits  = sensor.second;
    if (hits.size() == 0)
      continue;
    // find cellID for the cell with maximum ADC value within a sensor
    auto id            = hits[0].getCellID();
    auto max_charge    = hits[0].getEdep();
    auto earliest_time = hits[0].getTime();

    ROOT::VecOps::RVec<double> weights;
    for (const auto& hit : hits) {
      // weigh all hits by ADC value
      auto pos = m_seg->position(hit.getCellID());
      ave_x += hit.getEdep() * pos.x();
      ave_y += hit.getEdep() * pos.y();
      ave_z += hit.getEdep() * pos.z();

      tot_charge += hit.getEdep();
      if (hit.getEdep() > max_charge) {
        max_charge = hit.getEdep();
        id         = hit.getCellID();
      }
      earliest_time = std::min(earliest_time, hit.getTime());
      cluster.addToHits(hit);
      weights.push_back(hit.getEdep());
    }

    weights /= tot_charge;
    ave_x /= tot_charge;
    ave_y /= tot_charge;
    ave_z /= tot_charge;

    for (const auto& w : weights)
      cluster.addToWeights(w);

    edm4eic::Cov3f covariance;
    edm4hep::Vector2f locPos{static_cast<float>(ave_x / mm), static_cast<float>(ave_y / mm)};

    // CAUTION: surface has to be the cell where the cluster center belongs to
    // NOT the cell with MAX ADC value
    const auto* context = m_converter->findContext(id);
    auto gPos           = this->_local2Global(context, locPos);
    id                  = m_converter->cellID(gPos);

    cluster.setSurface(id);
    cluster.setLoc(locPos);
    cluster.setTime(earliest_time);
    cluster.setCovariance(covariance);
  }
}

} // namespace eicrecon
