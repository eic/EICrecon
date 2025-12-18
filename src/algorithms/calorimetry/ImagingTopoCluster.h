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
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
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
  std::array<double, 2> ScFi_sameLayerDistXY{0, 0};
  std::array<double, 2> ScFi_diffLayerDistXY{0, 0};
  std::array<double, 2> Img_sameLayerDistXY{0, 0};
  std::array<double, 2> Img_diffLayerDistXY{0, 0};
  std::array<double, 3> sameLayerDistXYZ{0, 0, 0};
  std::array<double, 3> diffLayerDistXYZ{0, 0, 0};
  std::array<double, 3> ScFi_sameLayerDistXYZ{0, 0, 0};
  std::array<double, 3> ScFi_diffLayerDistXYZ{0, 0, 0};
  std::array<double, 3> Img_sameLayerDistXYZ{0, 0, 0};
  std::array<double, 3> Img_diffLayerDistXYZ{0, 0, 0};
  std::array<double, 2> sameLayerDistEtaPhi{0, 0};
  std::array<double, 2> diffLayerDistEtaPhi{0, 0};
  std::array<double, 2> sameLayerDistTZ{0, 0};
  std::array<double, 2> diffLayerDistTZ{0, 0};
  std::array<double, 2> ScFi_sameLayerDistEtaPhi{0, 0};
  std::array<double, 2> ScFi_diffLayerDistEtaPhi{0, 0};
  std::array<double, 2> ScFi_sameLayerDistTZ{0, 0};
  std::array<double, 2> ScFi_diffLayerDistTZ{0, 0};
  std::array<double, 2> Img_sameLayerDistEtaPhi{0, 0};
  std::array<double, 2> Img_diffLayerDistEtaPhi{0, 0};
  std::array<double, 2> Img_sameLayerDistTZ{0, 0};
  std::array<double, 2> Img_diffLayerDistTZ{0, 0};

  std::array<double, 3> cross_system_DistXYZ{0, 0, 0};

  double sectorDist{0};
  double cross_system_sectorDist{0};
  double ScFi_sectorDist{0};
  double Img_sectorDist{0};
  double minClusterHitEdep{0};
  double minClusterCenterEdep{0};
  double minClusterEdep{0};

public:
  void init();
  void process(const Input& input, const Output& output) const final;

  // +++++++++++++++++++++++++++++++ based on sytem Id and across the system neighbouring ++++

private:
  // helper function to group hits

  // std::vector<std::vector<size_t>> mergeCrossSystemClusters(const std::map<int,
  //  std::vector<size_t>>& clusters_by_system,
  //  const std::vector<std::vector<size_t>>& all_clusters,
  //  const edm4eic::CalorimeterHitCollection& hits) const;

  bool cross_system_is_neighbour(const edm4eic::CalorimeterHit& h1,
                                 const edm4eic::CalorimeterHit& h2) const;
  bool is_neighbour(const edm4eic::CalorimeterHit& h1, const edm4eic::CalorimeterHit& h2) const;

  // Pointer to the geometry service
  dd4hep::IDDescriptor m_idSpec;

  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

  // grouping function with Breadth-First Search
  // note: template to allow Compare only known in local scope of caller
  template <typename Compare>
  void bfs_group(const edm4eic::CalorimeterHitCollection& hits,
                 std::set<std::size_t, Compare>& indices, std::list<std::size_t>& group,
                 std::vector<std::pair<size_t, size_t>>& edges, const std::size_t idx) const {

    auto* sys_field = m_idSpec.field("system");
    if (!sys_field) {
      error("Field 'system' not found in IDSpec for BFS grouping");
      return;
    }

    int sys = sys_field->value(hits[idx].getCellID());
    // debug("Starting BFS for hit {} in system {}", idx, sys);

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

        // debug("Checking neighbor for hit {} in system {}", *idx2, sys_field->value(hits[*idx2].getCellID()));

        // skip hits form other system
        if (sys_field->value(hits[*idx2].getCellID()) != sys) {
          debug("  Skipping hit {}: different system", *idx2);
          ++idx2;
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
          edges.emplace_back(*idx1, *idx2);
          // debug("hit {} and {} are neighbors", *idx1, *idx2);
          group.push_back(*idx2);
          idx2 = indices.erase(idx2); // takes role of idx2++
        } else {
          // debug("hit {} and {} are not neighbors", *idx1, *idx2);
          idx2++;
        }
      }
    }
  }
};

} // namespace eicrecon

// --------- Version 1 based on system Id but without cross sytem clustering -------------

// #pragma once

// #include <algorithms/algorithm.h>
// #include <DD4hep/Detector.h>
// #include <DD4hep/IDDescriptor.h>
// #include <algorithms/algorithm.h>
// #include <algorithms/geo.h>
// // Event Model related classes
// #include <edm4eic/CalorimeterHitCollection.h>
// #include <edm4eic/ProtoClusterCollection.h>
// #include <array>
// #include <cstddef>
// #include <list>
// #include <set>
// #include <string>
// #include <string_view>

// #include "ImagingTopoClusterConfig.h"
// #include "algorithms/interfaces/WithPodConfig.h"

// namespace eicrecon {

// using ImagingTopoClusterAlgorithm =
//     algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
//                           algorithms::Output<edm4eic::ProtoClusterCollection>>;

// class ImagingTopoCluster : public ImagingTopoClusterAlgorithm,
//                            public WithPodConfig<ImagingTopoClusterConfig> {

// public:
//   ImagingTopoCluster(std::string_view name)
//       : ImagingTopoClusterAlgorithm{
//             name,
//             {"inputHitCollection"},
//             {"outputProtoClusterCollection"},
//             "Topological cell clustering algorithm for imaging calorimetry."} {}

// private:
//   // unitless counterparts of the input parameters
//   std::array<double, 2> sameLayerDistXY{0, 0};
//   std::array<double, 2> diffLayerDistXY{0, 0};
//   std::array<double, 2> ScFi_sameLayerDistXY{0, 0};
//   std::array<double, 2> ScFi_diffLayerDistXY{0, 0};
//   std::array<double, 2> Img_sameLayerDistXY{0, 0};
//   std::array<double, 2> Img_diffLayerDistXY{0, 0};
//   std::array<double, 3> sameLayerDistXYZ{0, 0,0};
//   std::array<double, 3> diffLayerDistXYZ{0, 0,0};
//   std::array<double, 3> ScFi_sameLayerDistXYZ{0, 0,0};
//   std::array<double, 3> ScFi_diffLayerDistXYZ{0, 0,0};
//   std::array<double, 3> Img_sameLayerDistXYZ{0, 0,0};
//   std::array<double, 3> Img_diffLayerDistXYZ{0, 0,0};
//   std::array<double, 2> sameLayerDistEtaPhi{0, 0};
//   std::array<double, 2> diffLayerDistEtaPhi{0, 0};
//   std::array<double, 2> sameLayerDistTZ{0, 0};
//   std::array<double, 2> diffLayerDistTZ{0, 0};
//   std::array<double, 2> ScFi_sameLayerDistEtaPhi{0, 0};
//   std::array<double, 2> ScFi_diffLayerDistEtaPhi{0, 0};
//   std::array<double, 2> ScFi_sameLayerDistTZ{0, 0};
//   std::array<double, 2> ScFi_diffLayerDistTZ{0, 0};
//   std::array<double, 2> Img_sameLayerDistEtaPhi{0, 0};
//   std::array<double, 2> Img_diffLayerDistEtaPhi{0, 0};
//   std::array<double, 2> Img_sameLayerDistTZ{0, 0};
//   std::array<double, 2> Img_diffLayerDistTZ{0, 0};
//   double sectorDist{0};
//   double ScFi_sectorDist{0};
//   double Img_sectorDist{0};
//   double minClusterHitEdep{0};
//   double minClusterCenterEdep{0};
//   double minClusterEdep{0};

// public:
//   void init();
//   void process(const Input& input, const Output& output) const final;

// // +++++++++++++++++++++++++++++++ V1 : Without cross system neighbouring +++++++++++++++++++++++++++++++++

// private:
//   // helper function to group hits
//   bool is_neighbour(const edm4eic::CalorimeterHit& h1, const edm4eic::CalorimeterHit& h2) const;

//   // Pointer to the geometry service
//   dd4hep::IDDescriptor m_idSpec;

//   const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

//   // grouping function with Breadth-First Search
//   // note: template to allow Compare only known in local scope of caller
//   template <typename Compare>
//   void bfs_group(const edm4eic::CalorimeterHitCollection& hits,
//                  std::set<std::size_t, Compare>& indices, std::list<std::size_t>& group,
//                  const std::size_t idx) const {

//     auto* sys_field = m_idSpec.field("system");
//     if (!sys_field) {
//         error("Field 'system' not found in IDSpec for BFS grouping");
//         return;
//     }

//     int sys = sys_field->value(hits[idx].getCellID());
//     // debug("Starting BFS for hit {} in system {}", idx, sys);

//     // loop over group as it grows, until the end is stable and we reach it
//     for (auto idx1 = group.begin(); idx1 != group.end(); ++idx1) {
//       // check neighbours (note comments on loop over set above)
//       for (auto idx2 = indices.begin(); idx2 != indices.end();
//            indices.empty() ? idx2 = indices.end() : idx2) {

//         // skip idx1 and original idx
//         // (we cannot erase idx since it would invalidate iterator in calling scope)
//         if (*idx2 == *idx1 || *idx2 == idx) {
//           idx2++;
//           continue;
//         }

//         // debug("Checking neighbor for hit {} in system {}", *idx2, sys_field->value(hits[*idx2].getCellID()));

//         // skip hits form other system
//         if (sys_field->value(hits[*idx2].getCellID()) != sys) {
//                 debug("  Skipping hit {}: different system", *idx2);
//                 ++idx2;
//                 continue;
//         }

//         // skip rest of list of hits when we're past relevant layers
//         //if (hits[*idx2].getLayer() - hits[*idx1].getLayer() > m_cfg.neighbourLayersRange) {
//         //  break;
//         //}

//         // not energetic enough for cluster hit
//         if (hits[*idx2].getEnergy() < m_cfg.minClusterHitEdep) {
//           idx2 = indices.erase(idx2);
//           continue;
//         }

//         if (is_neighbour(hits[*idx1], hits[*idx2])) {
//           // debug("hit {} and {} are neighbors", *idx1, *idx2);
//           group.push_back(*idx2);
//           idx2 = indices.erase(idx2); // takes role of idx2++
//         } else {
//           // debug("hit {} and {} are not neighbors", *idx1, *idx2);
//           idx2++;
//         }
//       }
//     }
//    }
//   };

// } // namespace eicrecon

// ++++++++++++++++++++++++++++++ Version 2: Cross system Clustering ++++++++++++++++++++++++++++++++++

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
// #pragma once

// #include <algorithms/algorithm.h>
// #include <DD4hep/Detector.h>
// #include <DD4hep/IDDescriptor.h>
// #include <algorithms/geo.h>
// // Event Model related classes
// #include <edm4eic/CalorimeterHitCollection.h>
// #include <edm4eic/ProtoClusterCollection.h>
// #include <array>
// #include <cstddef>
// #include <list>
// #include <set>
// #include <string>
// #include <string_view>

// #include "ImagingTopoClusterConfig.h"
// #include "algorithms/interfaces/WithPodConfig.h"

// namespace eicrecon {

// using ImagingTopoClusterAlgorithm =
//     algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
//                           algorithms::Output<edm4eic::ProtoClusterCollection>>;

// class ImagingTopoCluster : public ImagingTopoClusterAlgorithm,
//                            public WithPodConfig<ImagingTopoClusterConfig> {

// public:
//   ImagingTopoCluster(std::string_view name)
//       : ImagingTopoClusterAlgorithm{
//             name,
//             {"inputHitCollection"},
//             {"outputProtoClusterCollection"},
//             "Topological cell clustering algorithm for imaging calorimetry."} {}

// private:
//   // unitless counterparts of the input parameters
//   std::array<double, 2> sameLayerDistXY{0, 0};
//   std::array<double, 2> diffLayerDistXY{0, 0};
//   std::array<double, 2> ScFi_sameLayerDistXY{0, 0};
//   std::array<double, 2> ScFi_diffLayerDistXY{0, 0};
//   std::array<double, 2> Img_sameLayerDistXY{0, 0};
//   std::array<double, 2> Img_diffLayerDistXY{0, 0};
//   std::array<double, 3> sameLayerDistXYZ{0, 0,0};
//   std::array<double, 3> diffLayerDistXYZ{0, 0,0};
//   std::array<double, 3> ScFi_sameLayerDistXYZ{0, 0,0};
//   std::array<double, 3> ScFi_diffLayerDistXYZ{0, 0,0};
//   std::array<double, 3> Img_sameLayerDistXYZ{0, 0,0};
//   std::array<double, 3> Img_diffLayerDistXYZ{0, 0,0};
//   std::array<double, 2> sameLayerDistEtaPhi{0, 0};
//   std::array<double, 2> diffLayerDistEtaPhi{0, 0};
//   std::array<double, 2> sameLayerDistTZ{0, 0};
//   std::array<double, 2> diffLayerDistTZ{0, 0};
//   std::array<double, 2> ScFi_sameLayerDistEtaPhi{0, 0};
//   std::array<double, 2> ScFi_diffLayerDistEtaPhi{0, 0};
//   std::array<double, 2> ScFi_sameLayerDistTZ{0, 0};
//   std::array<double, 2> ScFi_diffLayerDistTZ{0, 0};
//   std::array<double, 2> Img_sameLayerDistEtaPhi{0, 0};
//   std::array<double, 2> Img_diffLayerDistEtaPhi{0, 0};
//   std::array<double, 2> Img_sameLayerDistTZ{0, 0};
//   std::array<double, 2> Img_diffLayerDistTZ{0, 0};
//   double sectorDist{0};
//   double ScFi_sectorDist{0};
//   double Img_sectorDist{0};
//   double minClusterHitEdep{0};
//   double minClusterCenterEdep{0};
//   double minClusterEdep{0};

// public:
//   void init();
//   void process(const Input& input, const Output& output) const final;

// private:
//   // helper function to group hits
//   bool is_neighbour(const edm4eic::CalorimeterHit& h1, const edm4eic::CalorimeterHit& h2) const;

//   // Pointer to the geometry service
//   dd4hep::IDDescriptor m_idSpec;

//   const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

//   // grouping function with Breadth-First Search
//   // note: template to allow Compare only known in local scope of caller
//   template <typename Compare>
//   void bfs_group(const edm4eic::CalorimeterHitCollection& hits,
//                  std::set<std::size_t, Compare>& indices, std::list<std::size_t>& group,
//                  const std::size_t idx) const {

//     // loop over group as it grows, until the end is stable and we reach it
//     for (auto idx1 = group.begin(); idx1 != group.end(); ++idx1) {
//       // check neighbours (note comments on loop over set above)
//       for (auto idx2 = indices.begin(); idx2 != indices.end();
//            indices.empty() ? idx2 = indices.end() : idx2) {

//         // skip idx1 and original idx
//         // (we cannot erase idx since it would invalidate iterator in calling scope)
//         if (*idx2 == *idx1 || *idx2 == idx) {
//           idx2++;
//           continue;
//         }

//         // skip rest of list of hits when we're past relevant layers
//         //if (hits[*idx2].getLayer() - hits[*idx1].getLayer() > m_cfg.neighbourLayersRange) {
//         //  break;
//         //}

//         // not energetic enough for cluster hit
//         if (hits[*idx2].getEnergy() < m_cfg.minClusterHitEdep) {
//           idx2 = indices.erase(idx2);
//           continue;
//         }

//         if (is_neighbour(hits[*idx1], hits[*idx2])) {
//           group.push_back(*idx2);
//           // Print trace with system type
//           auto* sys_field = m_idSpec.field("system");
//           int sys1 = sys_field ? sys_field->value(hits[*idx1].getCellID()) : -1;
//           int sys2 = sys_field ? sys_field->value(hits[*idx2].getCellID()) : -1;

//           trace("Neighbor relation: hit {} (system {}) <-> hit {} (system {})",
//               *idx1, sys1, *idx2, sys2);

//           idx2 = indices.erase(idx2); // takes role of idx2++
//         } else {
//           idx2++;
//         }
//        }
//     }
//   }
// };

// } // namespace eicrecon
