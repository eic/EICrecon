// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024, Simon Gardner

#include <DD4hep/Handle.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JException.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <ROOT/RVec.hxx>
#include <algorithms/geo.h>
#include <edm4hep/Vector3d.h>
#include <fmt/core.h>
#include <gsl/pointers>
#include <sys/types.h>

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"
#include "algorithms/fardetectors/FarDetectorTrackerClusterConfig.h"

namespace eicrecon {

void FarDetectorTrackerCluster::init() {

  m_detector         = algorithms::GeoSvc::instance().detector();
  m_cellid_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();

  if (m_cfg.readout.empty()) {
    throw JException("Readout is empty");
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
    throw JException("Failed to load ID decoder");
  }
}

void FarDetectorTrackerCluster::process(const FarDetectorTrackerCluster::Input& input,
                                        const FarDetectorTrackerCluster::Output& output) const {

  const auto [inputHitsCollections] = input;
  auto [outputClustersCollection]   = output;

  // Loop over input and output collections - Any collection should only contain hits from a single
  // surface
  for (size_t i = 0; i < inputHitsCollections.size(); i++) {
    auto inputHits = inputHitsCollections[i];
    if (inputHits->size() == 0)
      continue;
    auto outputClusters = outputClustersCollection[i];

    // Make clusters
    auto clusters = ClusterHits(*inputHits);

    // Create TrackerHits from 2D cluster positions
    ConvertClusters(clusters, *outputClusters);
  }
}

// Create vector of FDTrackerCluster from list of hits
std::vector<FDTrackerCluster>
FarDetectorTrackerCluster::ClusterHits(const edm4eic::RawTrackerHitCollection& inputHits) const {

  std::vector<FDTrackerCluster> clusters;

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
    e.push_back(hit.getCharge());
    t.push_back(hit.getTimeStamp());
  }

  // Set up clustering variables
  ROOT::VecOps::RVec<bool> available(id.size(), 1);
  auto indices = Enumerate(id);

  // Loop while there are unclustered hits
  while (ROOT::VecOps::Any(available)) {

    dd4hep::Position localPos = {0, 0, 0};
    float weightSum           = 0;

    float esum    = 0;
    float t0      = 0;
    float tError  = 0;
    auto maxIndex = ROOT::VecOps::ArgMax(e * available);

    available[maxIndex] = 0;

    ROOT::VecOps::RVec<unsigned long> clusterList = {maxIndex};
    ROOT::VecOps::RVec<float> clusterT;
    std::vector<podio::ObjectID> clusterHits;

    // Loop over hits, adding neighbouring hits as relevant
    while (clusterList.size()) {

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

      // Adds raw hit to TrackerHit contribution
      clusterHits.push_back((inputHits)[index].getObjectID());

      // Energy
      auto hitE = e[index];
      esum += hitE;
      // TODO - See if now a single detector element is expected a better function is available.
      auto pos = m_seg->position(id[index]);

      // Weighted position
      float weight = hitE; // TODO - Calculate appropriate weighting based on sensor charge sharing
      weightSum += weight;
      localPos += pos * weight;

      // Time
      clusterT.push_back(t[index]);
    }

    // Finalise position
    localPos /= weightSum;

    // Finalise time
    t0     = Mean(clusterT);
    tError = StdDev(clusterT); // TODO fold detector timing resolution into error

    // Create cluster
    clusters.push_back(FDTrackerCluster{.cellID    = id[maxIndex],
                                        .x         = localPos.x(),
                                        .y         = localPos.y(),
                                        .energy    = esum,
                                        .time      = t0,
                                        .timeError = tError,
                                        .rawHits   = clusterHits});
  }

  return clusters;
}

// Convert to global coordinates and create TrackerHits
void FarDetectorTrackerCluster::ConvertClusters(
    const std::vector<FDTrackerCluster>& clusters,
    edm4hep::TrackerHitCollection& outputClusters) const {

  // Get context of first hit
  const dd4hep::VolumeManagerContext* context = m_cellid_converter->findContext(clusters[0].cellID);

  for (auto cluster : clusters) {
    auto hitPos = outputClusters.create();

    auto globalPos = context->localToWorld({cluster.x, cluster.y, 0});

    // Set cluster members
    hitPos.setCellID(cluster.cellID);
    hitPos.setPosition(edm4hep::Vector3d(globalPos.x() / dd4hep::mm, globalPos.y() / dd4hep::mm,
                                         globalPos.z() / dd4hep::mm));
    hitPos.setEDep(cluster.energy);
    hitPos.setTime(cluster.time);

    // Add raw hits to cluster
    for (auto hit : cluster.rawHits) {
      hitPos.addToRawHits(hit);
    }
  }
}

} // namespace eicrecon
