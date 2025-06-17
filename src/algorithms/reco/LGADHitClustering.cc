// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#include "LGADHitClustering.h"
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include "algorithms/interfaces/ActsSvc.h"
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
#include <fmt/core.h>
#include <algorithm>
#include <gsl/pointers>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/reco/LGADHitClusteringConfig.h"

namespace eicrecon {

void LGADHitClustering::init() {

  m_converter    = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_detector     = algorithms::GeoSvc::instance().detector();
  m_seg          = m_detector->readout(m_cfg.readout).segmentation();
  auto type      = m_seg.type();
  m_decoder      = m_seg.decoder();
  m_acts_context = algorithms::ActsSvc::instance().acts_geometry_provider();
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

void LGADHitClustering::_calcCluster(const Output& output, 
		const std::vector<edm4eic::TrackerHit>& hits, 
		size_t id,
		double timeWindow) const {
  constexpr double mm_acts = Acts::UnitConstants::mm;
  using dd4hep::mm;

  if (hits.size() == 0 || id >= hits.size())
    return;

  auto [clusters] = output;
  auto cluster    = clusters->create();
  // Right now the clustering algorithm a simple average over all hits in a sensors
  // Will be problematic near the edges, but it's just an illustration
  float ave_x = 0, ave_y = 0;
  float sigma2_x = 0, sigma2_y = 0;
  double tot_charge = 0;
  // find cellID for the cell with maximum ADC value within a sensor
  auto cellID     = hits[id].getCellID();
  auto max_charge = hits[id].getEdep();
  // hits vector is sorted by time. Therefore, the first entry is the earliest time
  auto earliest_time = hits[id].getTime();
  auto time_err      = hits[id].getTimeError();
  auto curr_time     = earliest_time; // check if hits are sorted by time

  ROOT::VecOps::RVec<double> weights;

  for (; id < hits.size(); ++id) {
    const auto& hit = hits[id];
    auto time       = hit.getTime();
    if (!(time >= curr_time))
      error("Hit time is moving backwards! Have you sorted sensors by time? Please do so if not.");
    else
      curr_time = time;

    if (curr_time - earliest_time > timeWindow) {
      // hits that are too far away in time from the earliest hit are considered a second cluster
      this->_calcCluster(clusters, hits, id, timeWindow);
      break;
    }
    // weigh all hits by ADC value
    auto pos = m_seg->position(hit.getCellID());
    if (hit.getEdep() < 0)
      error("Edep for hit at cellID{} is negative. Abort!.", hit.getCellID());
    const auto Edep = hit.getEdep();
    ave_x += Edep * pos.x();
    ave_y += Edep * pos.y();
    sigma2_x += Edep * Edep * hit.getPositionError().xx * mm_acts * mm_acts;
    sigma2_y += Edep * Edep * hit.getPositionError().yy * mm_acts * mm_acts;

    tot_charge += Edep;
    if (Edep > max_charge) {
      max_charge = Edep;
      cellID     = hit.getCellID();
    }
    cluster.addToHits(hit);
    weights.push_back(Edep);
  }

  weights /= tot_charge;
  ave_x /= tot_charge;
  ave_y /= tot_charge;
  sigma2_x /= tot_charge * tot_charge;
  sigma2_y /= tot_charge * tot_charge;

  // covariance copied from TrackerMeasurementFromHits.vv
  Acts::SquareMatrix2 cov = Acts::SquareMatrix2::Zero();
  cov(0, 0) = sigma2_x;
  cov(1, 1) = sigma2_y;
  cov(0, 1) = cov(1, 0) = 0.0;

  for (const auto& w : weights)
    cluster.addToWeights(w);

  edm4eic::Cov3f covariance;
  edm4hep::Vector2f locPos{static_cast<float>(ave_x / mm), static_cast<float>(ave_y / mm)};

  const auto* context    = m_converter->findContext(cellID);
  auto volID             = context->identifier;
  const auto& surfaceMap = m_acts_context->surfaceMap();
  const auto is          = surfaceMap.find(volID);
  if (is == surfaceMap.end())
    error(" WARNING: vol_id ({})  not found in m_surfaces.", volID);

  const Acts::Surface* surface = is->second;

  cluster.setSurface(surface->geometryId().value());
  cluster.setLoc(locPos);
  cluster.setTime(earliest_time);
  cluster.setCovariance({cov(0, 0), cov(1, 1), time_err * time_err,
                          cov(0, 1)}); // Covariance on location and time
}

void LGADHitClustering::process(const LGADHitClustering::Input& input,
                                const LGADHitClustering::Output& output) const {
  const auto [calibrated_hits] = input;

  // collection of ADC values from all sensors and group them by sensor
  std::unordered_map<dd4hep::rec::CellID, std::vector<edm4eic::TrackerHit>> hitsBySensors;

  for (const auto& calibrated_hit : *calibrated_hits) {

    auto id = calibrated_hit.getCellID();
    // Get sensors info
    auto sensorID = this->getSensorInfos(id);
    hitsBySensors[sensorID].emplace_back(calibrated_hit);
  }

  for (auto& [_, sensor] : hitsBySensors) {
    // sort content by time order for hit separation by time
    std::sort(sensor.begin(), sensor.end(),
              [](const edm4eic::TrackerHit& a, const edm4eic::TrackerHit& b) {
                return a.getTime() < b.getTime();
              });
    this->_calcCluster(output, sensor, 0, m_cfg.timeWindow);
  }
}

} // namespace eicrecon
