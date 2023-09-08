// Copyright (C) 2022, 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Thomas Britton
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <random>
#include <memory>

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>

#include <edm4hep/Vector2f.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/vector_utils.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <spdlog/spdlog.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "CalorimeterIslandClusterConfig.h"

namespace eicrecon {

  using CaloHit = edm4eic::CalorimeterHit;

  class CalorimeterIslandCluster : public WithPodConfig<CalorimeterIslandClusterConfig> {

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4eic::ProtoClusterCollection> process(const edm4eic::CalorimeterHitCollection &hits);

    // neighbor checking function
    std::function<edm4hep::Vector2f(const CaloHit&, const CaloHit&)> hitsDist;

    std::function<edm4hep::Vector2f(const CaloHit &h1, const CaloHit &h2)> transverseEnergyProfileMetric;
    double u_transverseEnergyProfileScale;
    double transverseEnergyProfileScaleUnits;

    // helper function to group hits
    std::function<bool(const CaloHit &h1, const CaloHit &h2)> is_neighbour;

    // unitless counterparts of the input parameters
    std::array<double, 2> neighbourDist;

    // Pointer to the geometry service
    dd4hep::IDDescriptor m_idSpec;

  private:

    static unsigned int function_id;

    // grouping function with Breadth-First Search
    void bfs_group(const edm4eic::CalorimeterHitCollection &hits, std::set<std::size_t> &group, std::size_t idx, std::vector<bool> &visits) const {
      visits[idx] = true;

      // not a qualified hit to particpate clustering, stop here
      if (hits[idx].getEnergy() < m_cfg.minClusterHitEdep) {
        return;
      }

      group.insert(idx);
      size_t prev_size = 0;

      while (prev_size != group.size()) {
        prev_size = group.size();
        for (std::size_t idx1 : group) {
          // check neighbours
          for (std::size_t idx2 = 0; idx2 < hits.size(); ++idx2) {
            // not a qualified hit to particpate clustering, skip
            if (hits[idx2].getEnergy() < m_cfg.minClusterHitEdep) {
              continue;
            }
            if ((!visits[idx2])
                && is_neighbour(hits[idx1], hits[idx2])) {
              group.insert(idx2);
              visits[idx2] = true;
            }
          }
        }
      }
    }

    // find local maxima that above a certain threshold
  std::vector<std::size_t> find_maxima(const edm4eic::CalorimeterHitCollection &hits, const std::set<std::size_t> &group, bool global = false) const {
    std::vector<std::size_t> maxima;
    if (group.empty()) {
      return maxima;
    }

    if (global) {
      std::size_t mpos = *group.begin();
      for (auto idx : group) {
        if (hits[mpos].getEnergy() < hits[idx].getEnergy()) {
          mpos = idx;
        }
      }
      if (hits[mpos].getEnergy() >= m_cfg.minClusterCenterEdep) {
        maxima.push_back(mpos);
      }
      return maxima;
    }

    for (std::size_t idx1 : group) {
      // not a qualified center
      if (hits[idx1].getEnergy() < m_cfg.minClusterCenterEdep) {
        continue;
      }

      bool maximum = true;
      for (std::size_t idx2 : group) {
        if (idx1 == idx2) {
          continue;
        }

        if (is_neighbour(hits[idx1], hits[idx2]) && (hits[idx2].getEnergy() > hits[idx1].getEnergy())) {
          maximum = false;
          break;
        }
      }

      if (maximum) {
        maxima.push_back(idx1);
      }
    }

    return maxima;
  }
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
    //TODO: confirm protoclustering without protoclustercollection
  void split_group(const edm4eic::CalorimeterHitCollection &hits, std::set<std::size_t>& group, const std::vector<std::size_t>& maxima, edm4eic::ProtoClusterCollection *protoClusters) const {
    // special cases
    if (maxima.empty()) {
      m_log->debug("No maxima found, not building any clusters");
      return;
    } else if (maxima.size() == 1) {
      edm4eic::MutableProtoCluster pcl = protoClusters->create();
      for (std::size_t idx : group) {
        pcl.addToHits(hits[idx]);
        pcl.addToWeights(1.);
      }

      m_log->debug("A single maximum found, added one ProtoCluster");

      return;
    }

    // split between maxima
    // TODO, here we can implement iterations with profile, or even ML for better splits
    std::vector<double> weights(maxima.size(), 1.);
    std::vector<edm4eic::MutableProtoCluster> pcls;
    for (size_t k = 0; k < maxima.size(); ++k) {
      pcls.push_back(protoClusters->create());
    }

    for (std::size_t idx : group) {
      size_t j = 0;
      // calculate weights for local maxima
      for (std::size_t cidx : maxima) {
        double energy   = hits[cidx].getEnergy();
        double dist     = edm4eic::magnitude(transverseEnergyProfileMetric(hits[cidx], hits[idx]));
        weights[j]      = std::exp(-dist * transverseEnergyProfileScaleUnits / m_cfg.transverseEnergyProfileScale) * energy;
        j += 1;
      }

      // normalize weights
      vec_normalize(weights);

      // ignore small weights
      for (auto& w : weights) {
        if (w < 0.02) {
          w = 0;
        }
      }
      vec_normalize(weights);

      // split energy between local maxima
      for (size_t k = 0; k < maxima.size(); ++k) {
        double weight = weights[k];
        if (weight <= 1e-6) {
          continue;
        }
        pcls[k].addToHits(hits[idx]);
        pcls[k].addToWeights(weight);
      }
    }
    m_log->debug("Multiple ({}) maxima found, added a ProtoClusters for each maximum", maxima.size());
  }
};

} // namespace eicrecon
