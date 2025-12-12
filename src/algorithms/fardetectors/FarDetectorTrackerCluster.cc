// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Simon Gardner

#include <DD4hep/Handle.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <ROOT/RVec.hxx>
#include <algorithms/geo.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <cstddef>
#include <gsl/pointers>

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"
#include "algorithms/fardetectors/FarDetectorTrackerClusterConfig.h"

namespace eicrecon {

void FarDetectorTrackerCluster::init() {

  m_detector = algorithms::GeoSvc::instance().detector();

  if (m_cfg.readout.empty()) {
    throw std::runtime_error("Readout is empty");
  }
  try {
    m_seg    = m_detector->readout(m_cfg.readout).segmentation();
    m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
    if (!m_cfg.x_field.empty()) {
      m_x_idx = m_id_dec->index(m_cfg.x_field);
      debug("Find layer field {}, index = {}", m_cfg.x_field, m_x_idx);
    }
    if (!m_cfg.y_field.empty()) {
      m_y_idx = m_id_dec->index(m_cfg.y_field);
      debug("Find layer field {}, index = {}", m_cfg.y_field, m_y_idx);
    }
  } catch (...) {
    error("Failed to load ID decoder for {}", m_cfg.readout);
    throw std::runtime_error("Failed to load ID decoder");
  }
}

void FarDetectorTrackerCluster::process(const FarDetectorTrackerCluster::Input& input,
                                        const FarDetectorTrackerCluster::Output& output) const {

  const auto [inputHitsCollections] = input;
  auto [outputClustersCollection]   = output;

  // Loop over input and output collections - Any collection should only contain hits from a single
  // surface
  for (std::size_t i = 0; i < inputHitsCollections.size(); i++) {
    auto inputHits = inputHitsCollections[i];
    if (inputHits->empty()) {
      continue;
    }
    auto outputClusters = outputClustersCollection[i];

    // Make clusters
    ClusterHits(*inputHits, *outputClusters);
  }
}

// Create vector of Measurement2D from list of hits
void FarDetectorTrackerCluster::ClusterHits(
    const edm4eic::TrackerHitCollection& inputHits,
    edm4eic::Measurement2DCollection& outputClusters) const {

  ROOT::VecOps::RVec<unsigned long> id;
  ROOT::VecOps::RVec<int> x;
  ROOT::VecOps::RVec<int> y;
  ROOT::VecOps::RVec<float> e;
  ROOT::VecOps::RVec<float> t;

  // Gather detector id positions
  for (const auto& hit : inputHits) {
    auto cellID = hit.getCellID();
    id.push_back(cellID);
    x.push_back(m_id_dec->get(cellID, m_x_idx));
    y.push_back(m_id_dec->get(cellID, m_y_idx));
    e.push_back(hit.getEdep());
    t.push_back(hit.getTime());
  }

  // Set up clustering variables
  ROOT::VecOps::RVec<bool> available(id.size(), 1);
  auto indices = Enumerate(id);

  // Loop while there are unclustered hits
  while (ROOT::VecOps::Any(available)) {

    dd4hep::Position localPos = {0, 0, 0};
    float weightSum           = 0;

    float t0      = 0;
    auto maxIndex = ROOT::VecOps::ArgMax(e * available);

    available[maxIndex] = 0;

    ROOT::VecOps::RVec<unsigned long> clusterList = {maxIndex};
    ROOT::VecOps::RVec<float> clusterT;
    ROOT::VecOps::RVec<float> clusterW;

    // Create cluster
    auto cluster = outputClusters.create();

    // Loop over hits, adding neighbouring hits as relevant
    while (!clusterList.empty()) {

      // Takes first remaining hit in cluster list
      auto index = clusterList[0];

      // Finds neighbours of cluster within time limit
      auto filter = available * (abs(x - x[index]) <= 1) * (abs(y - y[index]) <= 1) *
                    (abs(t - t[index]) < m_cfg.hit_time_limit);

      // Adds the found hits to the cluster
      clusterList = Concatenate(clusterList, indices[filter]);

      // Removes the found hits from the list of still available hits
      available = available * (!filter);

      // Removes current hit from remaining found cluster hits
      clusterList.erase(clusterList.begin());

      // TODO - See if now a single detector element is expected a better function is available.
      auto pos = m_seg->position(id[index]);

      // Weighted position
      float weight =
          e[index]; // TODO - Calculate appropriate weighting based on sensor charge sharing
      weightSum += weight;
      localPos += pos * weight;

      // Time
      clusterT.push_back(t[index]);

      // Adds hit and weight to Measurement2D contribution
      cluster.addToHits(inputHits[index]);
      clusterW.push_back(e[index]);
    }

    // Finalise position
    localPos /= weightSum;

    edm4hep::Vector2f xyPos = {static_cast<float>(localPos.x()), static_cast<float>(localPos.y())};

    // Finalise time
    t0 = Mean(clusterT);

    // Normalize weights then add to cluster
    clusterW /= weightSum;

    for (auto& w : clusterW) {
      cluster.addToWeights(w);
    }

    edm4eic::Cov3f covariance;

    cluster.setSurface(id[maxIndex]);
    cluster.setLoc(xyPos);
    cluster.setTime(t0);
    cluster.setCovariance(covariance);
  }
}

} // namespace eicrecon
