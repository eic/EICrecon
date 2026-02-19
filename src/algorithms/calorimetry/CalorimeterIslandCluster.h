// Copyright (C) 2022, 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Thomas Britton
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/Vector2f.h>
#include <array>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <list>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "CalorimeterIslandClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CaloHit = edm4eic::CalorimeterHit;

using CalorimeterIslandClusterAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
                          algorithms::Output<edm4eic::ProtoClusterCollection>>;

class CalorimeterIslandCluster : public CalorimeterIslandClusterAlgorithm,
                                 public WithPodConfig<CalorimeterIslandClusterConfig> {

public:
  CalorimeterIslandCluster(std::string_view name)
      : CalorimeterIslandClusterAlgorithm{name,
                                          {"inputProtoClusterCollection"},
                                          {"outputClusterCollection"},
                                          "Island clustering."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  std::vector<double> m_localDistXY;

public:
  // neighbor checking function
  std::function<edm4hep::Vector2f(const CaloHit&, const CaloHit&)> hitsDist;

  std::function<edm4hep::Vector2f(const CaloHit& h1, const CaloHit& h2)>
      transverseEnergyProfileMetric;
  double u_transverseEnergyProfileScale;
  double transverseEnergyProfileScaleUnits;

  // helper function to group hits
  std::function<bool(const CaloHit& h1, const CaloHit& h2)> is_neighbour;

  // helper function to define hit maximum
  std::function<bool(const CaloHit& maximum, const CaloHit& other)> is_maximum_neighbourhood;

  // unitless counterparts of the input parameters
  std::array<double, 2> neighbourDist;

  // Pointer to the geometry service
  dd4hep::IDDescriptor m_idSpec;

private:
  // grouping function with Breadth-First Search
  // note: template to allow Compare only known in local scope of caller
  template <typename Compare>
  void bfs_group(const edm4eic::CalorimeterHitCollection& hits,
                 std::set<std::size_t, Compare>& indices, std::list<std::size_t>& group,
                 const std::size_t idx) const {

    // loop over group as it grows, until the end is stable and we reach it
    for (auto idx1 = group.begin(); idx1 != group.end(); ++idx1) {
      // check neighbours (note comments on loop over set in process function)
      for (auto idx2 = indices.begin(); idx2 != indices.end();
           indices.empty() ? idx2 = indices.end() : idx2) {

        // skip idx1 and original idx
        // (we cannot erase idx since it would invalidate iterator in calling scope)
        if (*idx2 == *idx1 || *idx2 == idx) {
          idx2++;
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

  // find local maxima that above a certain threshold
  std::vector<std::size_t> find_maxima(const edm4eic::CalorimeterHitCollection& hits,
                                       const std::list<std::size_t>& group,
                                       bool global = false) const;

  // helper function
  inline static void vec_normalize(std::vector<double>& vals) {
    double total = 0.;
    for (auto& val : vals) {
      total += val;
    }
    for (auto& val : vals) {
      val /= total;
    }
  }

  // split a group of hits according to the local maxima
  void split_group(const edm4eic::CalorimeterHitCollection& hits, std::list<std::size_t>& group,
                   const std::vector<std::size_t>& maxima,
                   edm4eic::ProtoClusterCollection* protoClusters) const;
};

} // namespace eicrecon
