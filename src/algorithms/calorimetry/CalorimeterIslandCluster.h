// Copyright (C) 2022, 2023 Chao Peng, Wouter Deconinck, Sylvester Joosten, Thomas Britton
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <random>
#include <memory>

#include "services/geometry/dd4hep/JDD4hep_service.h"

#include <Evaluator/DD4hepUnits.h>

#include <edm4hep/Vector2f.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/vector_utils.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <spdlog/spdlog.h>

using CaloHit = edm4eic::CalorimeterHit;

class CalorimeterIslandCluster {

    // Insert any member variables here

public:
    CalorimeterIslandCluster() = default;
    void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun();
    std::unique_ptr<edm4eic::ProtoClusterCollection> AlgorithmProcess(const edm4eic::CalorimeterHitCollection &hits);

    //-------- Configuration Parameters ------------
    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_log;

    std::string m_input_tag;

    // geometry service to get ids
    std::string m_geoSvcName; //{this, "geoServiceName", "GeoSvc"};
    std::string m_readout; //{this, "readoutClass", ""};
    std::string u_adjacencyMatrix; //{this, "adjacencyMatrix", ""};

    // neighbour checking distances
    double m_sectorDist;//{this, "sectorDist", 5.0 * dd4hep::cm};
    std::vector<double> u_localDistXY;//{this, "localDistXY", {}};
    std::vector<double> u_localDistXZ;//{this, "localDistXZ", {}};
    std::vector<double> u_localDistYZ;//{this, "localDistYZ", {}};
    std::vector<double> u_globalDistRPhi;//{this, "globalDistRPhi", {}};
    std::vector<double> u_globalDistEtaPhi;//{this, "globalDistEtaPhi", {}};
    std::vector<double> u_dimScaledLocalDistXY;//{this, "dimScaledLocalDistXY", {1.8, 1.8}};
    // neighbor checking function
    std::function<edm4hep::Vector2f(const CaloHit&, const CaloHit&)> hitsDist;

    bool m_splitCluster = false;
    double m_minClusterHitEdep;
    double m_minClusterCenterEdep;
    std::string u_transverseEnergyProfileMetric;
    std::function<edm4hep::Vector2f(const CaloHit &h1, const CaloHit &h2)> transverseEnergyProfileMetric;
    double u_transverseEnergyProfileScale;
    double transverseEnergyProfileScaleUnits;

    // helper function to group hits
    std::function<bool(const CaloHit &h1, const CaloHit &h2)> is_neighbour;

    // unitless counterparts of the input parameters
    std::array<double, 2> neighbourDist;

    // Pointer to the geometry service
    std::shared_ptr<JDD4hep_service> m_geoSvc;
    dd4hep::IDDescriptor m_idSpec;

    //-----------------------------------------------

    // unitless counterparts of inputs
    double           stepTDC, tRes, eRes[3];
    //Rndm::Numbers    m_normDist;
    uint64_t         id_mask, ref_mask;

private:
    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

   // grouping function with Depth-First Search
   //TODO: confirm grouping without calohitcollection
    void dfs_group(const edm4eic::CalorimeterHitCollection &hits, std::vector<std::size_t> &group, std::size_t idx, std::vector<bool> &visits) const {
        // not a qualified hit to particpate clustering, stop here
        if (hits[idx].getEnergy() < m_minClusterHitEdep) {
            visits[idx] = true;
            return;
        }
        group.push_back(idx);
        visits[idx] = true;
        for (size_t i = 0; i < hits.size(); ++i) {
            if (visits[i] || !is_neighbour(hits[idx], hits[i])) {
                continue;
            }
            dfs_group(hits, group, i, visits);
        }
    }

    // find local maxima that above a certain threshold
  std::vector<std::size_t> find_maxima(const edm4eic::CalorimeterHitCollection &hits, const std::vector<std::size_t> &group, bool global = false) const {
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
      if (hits[mpos].getEnergy() >= m_minClusterCenterEdep) {
        maxima.push_back(mpos);
      }
      return maxima;
    }

    for (std::size_t idx1 : group) {
      // not a qualified center
      if (hits[idx1].getEnergy() < m_minClusterCenterEdep) {
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
  void split_group(const edm4eic::CalorimeterHitCollection &hits, std::vector<std::size_t>& group, const std::vector<std::size_t>& maxima, edm4eic::ProtoClusterCollection *protoClusters) const {
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
    for (size_t k = 0; k < maxima.size(); ++k) {
      protoClusters->create();
    }

    for (std::size_t idx : group) {
      size_t j = 0;
      // calculate weights for local maxima
      for (std::size_t cidx : maxima) {
        double energy   = hits[cidx].getEnergy();
        double dist     = edm4eic::magnitude(transverseEnergyProfileMetric(hits[cidx], hits[idx]));
        weights[j]      = std::exp(-dist * transverseEnergyProfileScaleUnits / u_transverseEnergyProfileScale) * energy;
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
        (*protoClusters)[k].addToHits(hits[idx]);
        (*protoClusters)[k].addToWeights(weight);
      }
    }
    m_log->debug("Multiple ({}) maxima found, added a ProtoClusters for each maximum", maxima.size());
  }
};
