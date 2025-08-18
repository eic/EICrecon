// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

/*
 *  Topological Cell Clustering Algorithm for Imaging Calorimetry
 *  1. group all the adjacent pixels
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 *  Original reference: https://arxiv.org/pdf/1603.02934.pdf
 *
 *  Modifications:
 *
 *  Wouter Deconinck (Manitoba), 08/24/2024
 *  - converted hit storage model from std::vector to std::set sorted on layer
 *    where only hits remaining to be assigned to a group are in the set
 *  - erase hits that are too low in energy to be part of a cluster
 *  - converted group storage model from std::set to std::list to allow adding
 *    hits while keeping iterators valid
 *
 */
#pragma once

#include <algorithms/algorithm.h>
// Event Model related classes
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <array>
#include <cstddef>
#include <list>
#include <set>
#include <string>
#include <string_view>

#include "ImagingTopoClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ImagingTopoClusterAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
                          algorithms::Output<edm4eic::ProtoClusterCollection>>;

class ImagingTopoCluster : public ImagingTopoClusterAlgorithm,
                           public WithPodConfig<ImagingTopoClusterConfig> {

public:
  ImagingTopoCluster(std::string_view name)
      : ImagingTopoClusterAlgorithm{
            name,
            {"inputHitCollection"},
            {"outputProtoClusterCollection"},
            "Topological cell clustering algorithm for imaging calorimetry."} {}

private:
  // unitless counterparts of the input parameters
  std::array<double, 2> sameLayerDistXY{0, 0};
  std::array<double, 2> diffLayerDistXY{0, 0};
  std::array<double, 2> sameLayerDistEtaPhi{0, 0};
  std::array<double, 2> diffLayerDistEtaPhi{0, 0};
  std::array<double, 2> sameLayerDistPhiZ{0, 0};
  std::array<double, 2> diffLayerDistPhiZ{0, 0};
  double sectorDist{0};
  double minClusterHitEdep{0};
  double minClusterCenterEdep{0};
  double minClusterEdep{0};

public:
  void init();
  void process(const Input& input, const Output& output) const final;

private:
  // helper function to group hits
  bool is_neighbour(const edm4eic::CalorimeterHit& h1, const edm4eic::CalorimeterHit& h2) const;

  // grouping function with Breadth-First Search
  // note: template to allow Compare only known in local scope of caller
  template <typename Compare>
  void bfs_group(const edm4eic::CalorimeterHitCollection& hits,
                 std::set<std::size_t, Compare>& indices, std::list<std::size_t>& group,
                 const std::size_t idx) const {

    // loop over group as it grows, until the end is stable and we reach it
    for (auto idx1 = group.begin(); idx1 != group.end(); ++idx1) {
      // check neighbours (note comments on loop over set above)
      for (auto idx2 = indices.begin(); idx2 != indices.end();
           indices.empty() ? idx2 = indices.end() : idx2) {

        // skip idx1 and original idx
        // (we cannot erase idx since it would invalidate iterator in calling scope)
        if (*idx2 == *idx1 || *idx2 == idx) {
          idx2++;
          continue;
        }

        // skip rest of list of hits when we're past relevant layers
        //if (hits[*idx2].getLayer() - hits[*idx1].getLayer() > m_cfg.neighbourLayersRange) {
        //  break;
        //}

        // not energetic enough for cluster hit
        if (hits[*idx2].getEnergy() < m_cfg.minClusterHitEdep) {
          idx2 = indices.erase(idx2);
          continue;
        }

        if (is_neighbour(hits[*idx1], hits[*idx2])) {
          group.push_back(*idx2);
          idx2 = indices.erase(idx2); // takes role of idx2++
        } else {
          idx2++;
        }
      }
    }
  }
};

} // namespace eicrecon
