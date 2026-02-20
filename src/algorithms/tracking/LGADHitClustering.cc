// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#include "LGADHitClustering.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <DD4hep/Readout.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/Segmentation.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <ROOT/RVec.hxx>
#include <algorithms/geo.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector2f.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <set>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ActsDD4hepDetector.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/tracking/LGADHitClusteringConfig.h"

namespace eicrecon {

void LGADHitClustering::init() {

  m_converter     = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_detector      = algorithms::GeoSvc::instance().detector();
  m_seg           = m_detector->readout(m_cfg.readout).segmentation();
  auto type       = m_seg.type();
  m_decoder       = m_seg.decoder();
  m_acts_detector = algorithms::ActsSvc::instance().detector();
}

void LGADHitClustering::_calcCluster(const Output& output,
                                     const std::vector<edm4eic::TrackerHit>& hits) const {
  if (hits.empty()) {
    return;
  }
  constexpr double mm_acts = Acts::UnitConstants::mm;
  using dd4hep::mm;

  auto [clusters] = output;
  auto cluster    = clusters->create();
  // Right now the clustering algorithm is either:
  // 1. simple average over all hits in a sensors
  // 2. Cell position with max ADC value in a cluster
  // Switch between option 1 and 2 with m_cfg.useAve
  // Will be problematic near the edges, but it's just an illustration
  float ave_x = 0, ave_y = 0;
  float sigma2_x = 0, sigma2_y = 0;
  double tot_charge = 0;
  // find cellID for the cell with maximum ADC value within a sensor
  dd4hep::rec::CellID cellID = 0;
  auto max_charge            = std::numeric_limits<float>::min();
  auto earliest_time         = std::numeric_limits<float>::max();
  float time_err{0};
  float max_charge_x{0};
  float max_charge_y{0};
  float max_charge_sigma2_x{0};
  float max_charge_sigma2_y{0};

  ROOT::VecOps::RVec<double> weights;

  for (size_t id = 0; id < hits.size(); ++id) {
    const auto& hit = hits[id];
    if (hit.getTime() < earliest_time) {
      earliest_time = hit.getTime();
      time_err      = hit.getTimeError();
    }
    // weigh all hits by ADC value
    auto pos = m_seg->position(hit.getCellID());
    if (hit.getEdep() < 0) {
      error("Edep for hit at cellID{} is negative. Please check the accuracy of your energy "
            "calibration. ",
            hit.getCellID());
    }
    const auto Edep = hit.getEdep();
    ave_x += Edep * pos.x();
    ave_y += Edep * pos.y();
    sigma2_x += Edep * Edep * hit.getPositionError().xx * mm_acts * mm_acts;
    sigma2_y += Edep * Edep * hit.getPositionError().yy * mm_acts * mm_acts;

    tot_charge += Edep;
    if (Edep > max_charge) {
      max_charge          = Edep;
      cellID              = hit.getCellID();
      max_charge_x        = pos.x();
      max_charge_y        = pos.y();
      max_charge_sigma2_x = hit.getPositionError().xx * mm_acts * mm_acts;
      max_charge_sigma2_y = hit.getPositionError().yy * mm_acts * mm_acts;
    }
    cluster.addToHits(hit);
    weights.push_back(Edep);
  }

  if (m_cfg.useAve) {
    weights /= tot_charge;
    ave_x /= tot_charge;
    ave_y /= tot_charge;
    sigma2_x /= tot_charge * tot_charge;
    sigma2_y /= tot_charge * tot_charge;
  } else {
    ave_x    = max_charge_x;
    ave_y    = max_charge_y;
    sigma2_x = max_charge_sigma2_x;
    sigma2_y = max_charge_sigma2_y;
  }

  // covariance copied from TrackerMeasurementFromHits.cc
  Acts::SquareMatrix2 cov = Acts::SquareMatrix2::Zero();
  cov(0, 0)               = sigma2_x;
  cov(1, 1)               = sigma2_y;
  cov(0, 1) = cov(1, 0) = 0.0;

  for (const auto& w : weights) {
    cluster.addToWeights(w);
  }

  edm4eic::Cov3f covariance;
  edm4hep::Vector2f locPos{static_cast<float>(ave_x / mm), static_cast<float>(ave_y / mm)};

  const auto* context    = m_converter->findContext(cellID);
  auto volID             = context->identifier;
  const auto& surfaceMap = m_acts_detector->surfaceMap();
  const auto is          = surfaceMap.find(volID);
  if (is == surfaceMap.end()) {
    error("vol_id ({})  not found in m_surfaces.", volID);
  }

  const Acts::Surface* surface = is->second;

  cluster.setSurface(surface->geometryId().value());
  cluster.setLoc(locPos);
  cluster.setTime(earliest_time);
  cluster.setCovariance(
      {cov(0, 0), cov(1, 1), time_err * time_err, cov(0, 1)}); // Covariance on location and time
}

void LGADHitClustering::process(const LGADHitClustering::Input& input,
                                const LGADHitClustering::Output& output) const {
  const auto [calibrated_hits] = input;

  // use unordered map to efficiently search for hits by CellID
  // store the index of hits instead of the hit itself
  // UnionFind can only group integer objects, not edm4eic::TrackerHit
  std::unordered_map<dd4hep::rec::CellID, std::vector<int>> hitIDsByCells;

  for (size_t hitID = 0; hitID < calibrated_hits->size(); ++hitID) {
    hitIDsByCells[calibrated_hits->at(hitID).getCellID()].push_back(hitID);
  }

  // merge neighbors by union find
  UnionFind uf(static_cast<int>(calibrated_hits->size()));
  for (auto [cellID, hitIDs] : hitIDsByCells) {
    // code copied from SiliconChargeSharing for neighbor finding
    const auto* element = &m_converter->findContext(cellID)->element; // volume context
    auto [segmentationIt, segmentationInserted] =
        m_segmentation_map.try_emplace(element, getLocalSegmentation(cellID));

    std::set<dd4hep::rec::CellID> cellNeighbors;
    segmentationIt->second->neighbours(cellID, cellNeighbors);
    // find if there are hits in neighboring cells
    for (const auto& neighborCandidates : cellNeighbors) {
      auto it = hitIDsByCells.find(neighborCandidates);
      if (it != hitIDsByCells.end()) {
        for (const auto& hitID1 : hitIDs) {
          for (const auto& hitID2 : it->second) {
            const auto& hit1 = calibrated_hits->at(hitID1);
            const auto& hit2 = calibrated_hits->at(hitID2);
            // only consider hits with time difference < deltaT as the same cluster
            if (std::fabs(hit1.getTime() - hit2.getTime()) < m_cfg.deltaT) {
              uf.merge(hitID1, hitID2);
            }
          }
        }
      }
    }
  }

  // group hits by cluster parent index according to union find algorithm
  std::unordered_map<int, std::vector<edm4eic::TrackerHit>> clusters;
  for (size_t hitID = 0; hitID < calibrated_hits->size(); ++hitID) {
    clusters[uf.find(hitID)].push_back(calibrated_hits->at(hitID));
  }

  // calculated weighted averages
  for (auto& [_, cluster] : clusters) {
    this->_calcCluster(output, cluster);
  }
}

// copied from SiliconChargeSharing
// Get the segmentation relevant to a cellID
const dd4hep::DDSegmentation::CartesianGridXY*
LGADHitClustering::getLocalSegmentation(const dd4hep::rec::CellID& cellID) const {
  // Get the segmentation type
  auto segmentation_type                                   = m_seg.type();
  const dd4hep::DDSegmentation::Segmentation* segmentation = m_seg.segmentation();
  // Check if the segmentation is a multi-segmentation
  while (segmentation_type == "MultiSegmentation") {
    const auto* multi_segmentation =
        dynamic_cast<const dd4hep::DDSegmentation::MultiSegmentation*>(segmentation);
    segmentation      = &multi_segmentation->subsegmentation(cellID);
    segmentation_type = segmentation->type();
  }

  // Try to cast the segmentation to CartesianGridXY
  const auto* cartesianGrid =
      dynamic_cast<const dd4hep::DDSegmentation::CartesianGridXY*>(segmentation);
  if (cartesianGrid == nullptr) {
    throw std::runtime_error("Segmentation is not of type CartesianGridXY");
  }

  return cartesianGrid;
}

LGADHitClustering::UnionFind::UnionFind(int n) : mParent(n, 0), mRank(n, 0) {
  for (int i = 0; i < n; ++i) {
    mParent[i] = i;
  }
}

int LGADHitClustering::UnionFind::find(int id) {
  if (mParent[id] == id) {
    return id;
  }
  return mParent[id] = find(mParent[id]); // path compression
}

void LGADHitClustering::UnionFind::merge(int id1, int id2) {
  auto root1 = find(id1);
  auto root2 = find(id2);

  if (root1 != root2) {
    if (mRank[root1] > mRank[root2]) {
      mParent[root2] = root1;
    } else if (mRank[root1] < mRank[root2]) {
      mParent[root1] = root2;
    } else {
      mParent[root1] = root2;
      mRank[root2]++;
    }
  }
}

} // namespace eicrecon
