// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
//#include <services/randomgenerator/randomGenerator.h>
#include <Evaluator/DD4hepUnits.h>

#include <edm4hep/Vector2f.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/vector_utils.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/MutableProtoCluster.h>
#include <spdlog/spdlog.h>

using CaloHit = edm4eic::CalorimeterHit;

static double Phi_mpi_pi(double phi) {
  return std::remainder(phi, 2 * M_PI);
}

//TODO:Reconcile edm4hep::Vector2f and edm4eic::Vector3f especially with regards to the operators and sign convention
static edm4hep::Vector2f localDistXY(const CaloHit *h1, const CaloHit *h2) {
  //edm4eic::Vector3f h1_pos=geo_converter->position(h1.getCellID());
  //edm4eic::Vector3f h2_pos=geo_converter->position(h2.getCellID());
  const auto delta =h1->getLocal() - h2->getLocal();
  return {delta.x, delta.y};
  //const auto deltax = h1.getLocal()[0] - h2.getLocal()[0];
  //const auto deltay = h1.getLocal()[1] - h2.getLocal()[1];
  //return {delta.x, delta.y,0};
  //return {deltax,deltay};
}
static edm4hep::Vector2f localDistXZ(const CaloHit *h1, const CaloHit *h2) {
  const auto delta = h1->getLocal() - h2->getLocal();
  //const auto deltax = h1.getLocal()[0] - h2.getLocal()[0];
  //const auto deltaz = h1.getLocal()[2] - h2.getLocal()[2];
  return {delta.x, delta.z};
}
static edm4hep::Vector2f localDistYZ(const CaloHit *h1, const CaloHit *h2) {
  const auto delta = h1->getLocal() - h2->getLocal();
  //const auto deltay = h1.getLocal()[1] - h2.getLocal()[1];
  //const auto deltaz = h1.getLocal()[2] - h2.getLocal()[2];
  return {delta.y, delta.z};
}
static edm4hep::Vector2f dimScaledLocalDistXY(const CaloHit *h1, const CaloHit *h2) {
  const auto delta = h1->getLocal() - h2->getLocal();
  //const auto deltax = h1.getLocal()[0] - h2.getLocal()[0];
  //const auto deltay = h1.getLocal()[1] - h2.getLocal()[1];

  const auto dimsum = h1->getDimension() + h2->getDimension();
  //const auto dimsumx = h1.getDimension()[0] + h2.getDimension()[0];
  //const auto dimsumy = h1.getDimension()[1] + h2.getDimension()[1];

  return {2 * delta.x / dimsum.x, 2 * delta.y / dimsum.y};
}
static edm4hep::Vector2f globalDistRPhi(const CaloHit *h1, const CaloHit *h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {
    static_cast<vector_type>(
      edm4eic::magnitude(h1->getPosition()) - edm4eic::magnitude(h2->getPosition())
    ),
    static_cast<vector_type>(
      Phi_mpi_pi(edm4eic::angleAzimuthal(h1->getPosition()) - edm4eic::angleAzimuthal(h2->getPosition()))
    )
  };
}
static edm4hep::Vector2f globalDistEtaPhi(const CaloHit *h1,
                                       const CaloHit *h2) {
  using vector_type = decltype(edm4hep::Vector2f::a);
  return {
    static_cast<vector_type>(
      edm4eic::eta(h1->getPosition()) - edm4eic::eta(h2->getPosition())
    ),
    static_cast<vector_type>(
      Phi_mpi_pi(edm4eic::angleAzimuthal(h1->getPosition()) - edm4eic::angleAzimuthal(h2->getPosition()))
    )
  };
}

class CalorimeterIslandCluster {

    // Insert any member variables here

public:
    CalorimeterIslandCluster() = default;
    virtual ~CalorimeterIslandCluster(){} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

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
    std::function<edm4hep::Vector2f(const CaloHit*, const CaloHit*)> hitsDist;

    bool m_splitCluster;
    double m_minClusterHitEdep;
    double m_minClusterCenterEdep;

    // helper function to group hits
    std::function<bool(const CaloHit* h1, const CaloHit* h2)> is_neighbour;

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

    // inputs/outputs
    std::vector<const edm4eic::CalorimeterHit*> hits;
    std::vector<edm4eic::ProtoCluster*> protoClusters;

private:
    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

   // grouping function with Depth-First Search
   //TODO: confirm grouping without calohitcollection
    void dfs_group(std::vector<std::pair<uint32_t, const CaloHit*>>& group, int idx,
                 std::vector<const CaloHit*> hits, std::vector<bool>& visits) const {
        // not a qualified hit to particpate clustering, stop here
        if (hits[idx]->getEnergy() < m_minClusterHitEdep) {
            visits[idx] = true;
            return;
        }
        group.emplace_back(idx, hits[idx]);
        visits[idx] = true;
        for (size_t i = 0; i < hits.size(); ++i) {
            if (visits[i] || !is_neighbour(hits[idx], hits[i])) {
                continue;
            }
            dfs_group(group, i, hits, visits);
        }
    }

    // find local maxima that above a certain threshold
  std::vector<const CaloHit*> find_maxima(const std::vector<std::pair<uint32_t, const CaloHit*>>& group,
              bool global = false) const {
    std::vector<const CaloHit*> maxima;
    if (group.empty()) {
      return maxima;
    }

    if (global) {
      int mpos = 0;
      for (size_t i = 0; i < group.size(); ++i) {
        if (group[mpos].second->getEnergy() < group[i].second->getEnergy()) {
          mpos = i;
        }
      }
      if (group[mpos].second->getEnergy() >= m_minClusterCenterEdep) {
        maxima.push_back(group[mpos].second);
      }
      return maxima;
    }

    for (const auto& [idx, hit] : group) {
      // not a qualified center
      if (hit->getEnergy() < m_minClusterCenterEdep) {
        continue;
      }

      bool maximum = true;
      for (const auto& [idx2, hit2] : group) {
        if (*hit == *hit2) {
          continue;
        }

        if (is_neighbour(hit, hit2) && (hit2->getEnergy() > hit->getEnergy())) {
          maximum = false;
          break;
        }
      }

      if (maximum) {
        maxima.push_back(hit);
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
  void split_group(std::vector<std::pair<uint32_t, const CaloHit*>>& group, const std::vector<const CaloHit*>& maxima,

                   std::vector<edm4eic::ProtoCluster *>& proto) const {
    // special cases
    if (maxima.empty()) {
      m_log->debug("No maxima found, not building any clusters");
      return;
    } else if (maxima.size() == 1) {
      edm4eic::MutableProtoCluster pcl;
      for (auto& [idx, hit] : group) {
        pcl.addToHits(*hit);
        pcl.addToWeights(1.);
      }
      proto.push_back(new edm4eic::ProtoCluster(pcl)); // TODO: Should we be using clone() here?

      m_log->debug("A single maximum found, added one ProtoCluster");

      return;
    }

    // split between maxima
    // TODO, here we can implement iterations with profile, or even ML for better splits
    std::vector<double> weights(maxima.size(), 1.);
    std::vector<edm4eic::MutableProtoCluster> pcls;
    for (size_t k = 0; k < maxima.size(); ++k) {
      pcls.emplace_back();
    }

    size_t i = 0;
    for (const auto& [idx, hit] : group) {
      size_t j = 0;
      // calculate weights for local maxima
      for (const auto& chit : maxima) {
        double dist_ref = chit->getDimension().x;
        double energy   = chit->getEnergy();
        double dist     = edm4eic::magnitude(hitsDist(chit, hit));
        weights[j]      = std::exp(-dist / dist_ref) * energy;
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
        pcls[k].addToHits(*hit);
        pcls[k].addToWeights(weight);
      }
      i += 1;
    }
    for (auto& pcl : pcls) {
      proto.push_back(new edm4eic::ProtoCluster(pcl)); // TODO: Should we be using clone() here?
    }
    m_log->debug("Multiple ({}) maxima found, added a ProtoClusters for each maximum", maxima.size());
  }
};
