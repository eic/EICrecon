// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

/*
 *  Topological Cell Clustering Algorithm for Imaging Calorimetry
 *  1. group all the adjacent pixels
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 *  References: https://arxiv.org/pdf/1603.02934.pdf
 *
 */
#pragma once

#include <algorithm>

#include <DD4hep/BitFieldCoder.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/vector_utils.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "ImagingTopoClusterConfig.h"

namespace eicrecon {

/** Topological Cell Clustering Algorithm.
 *
 * Topological Cell Clustering Algorithm for Imaging Calorimetry
 *  1. group all the adjacent pixels
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 *  References: https://arxiv.org/pdf/1603.02934.pdf
 *
 * \ingroup reco
 */
  class ImagingTopoCluster : public WithPodConfig<ImagingTopoClusterConfig> {

  protected:

    std::shared_ptr<spdlog::logger> m_log;

    // unitless counterparts of the input parameters
    double localDistXY[2]{0, 0}, layerDistEtaPhi[2]{0, 0}, sectorDist{0};
    double minClusterHitEdep{0}, minClusterCenterEdep{0}, minClusterEdep{0}, minClusterNhits{0};

  public:
    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;

        // unitless conversion
        // sanity checks
        if (m_cfg.localDistXY.size() != 2) {
            m_log->error( "Expected 2 values (x_dist, y_dist) for localDistXY");
            return;
        }
        if (m_cfg.layerDistEtaPhi.size() != 2) {
            m_log->error( "Expected 2 values (eta_dist, phi_dist) for layerDistEtaPhi" );
            return;
        }

        // using juggler internal units (GeV, dd4hep::mm, dd4hep::ns, dd4hep::rad)
        localDistXY[0] = m_cfg.localDistXY[0] / dd4hep::mm;
        localDistXY[1] = m_cfg.localDistXY[1] / dd4hep::mm;
        layerDistEtaPhi[0] = m_cfg.layerDistEtaPhi[0];
        layerDistEtaPhi[1] = m_cfg.layerDistEtaPhi[1] / dd4hep::rad;
        sectorDist = m_cfg.sectorDist / dd4hep::mm;
        minClusterHitEdep = m_cfg.minClusterHitEdep / dd4hep::GeV;
        minClusterCenterEdep = m_cfg.minClusterCenterEdep / dd4hep::GeV;
        minClusterEdep = m_cfg.minClusterEdep / dd4hep::GeV;

        // summarize the clustering parameters
        m_log->info("Local clustering (same sector and same layer): "
                    "Local [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
                    localDistXY[0], localDistXY[1]
        );
        m_log->info("Neighbour layers clustering (same sector and layer id within +- {:d}: "
                    "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
                    m_cfg.neighbourLayersRange, layerDistEtaPhi[0], layerDistEtaPhi[1]
        );
        m_log->info("Neighbour sectors clustering (different sector): "
                    "Global distance between hits <= {:.4f} mm.",
                    sectorDist
        );
    }

    std::unique_ptr<edm4eic::ProtoClusterCollection> process(const edm4eic::CalorimeterHitCollection& hits) {

        auto proto = std::make_unique<edm4eic::ProtoClusterCollection>();

        // group neighbouring hits
        std::vector<bool> visits(hits.size(), false);
        std::vector<std::set<std::size_t>> groups;
        for (size_t i = 0; i < hits.size(); ++i) {
            m_log->debug("hit {:d}: local position = ({}, {}, {}), global position = ({}, {}, {})", i + 1,
                         hits[i].getLocal().x, hits[i].getLocal().y, hits[i].getPosition().z,
                         hits[i].getPosition().x, hits[i].getPosition().y, hits[i].getPosition().z
            );
            // already in a group, or not energetic enough to form a cluster
            if (visits[i] || hits[i].getEnergy() < minClusterCenterEdep) {
                continue;
            }
            // create a new group, and group all the neighbouring hits
            groups.emplace_back();
            bfs_group(hits, groups.back(), i, visits);
        }
        m_log->debug("found {} potential clusters (groups of hits)", groups.size());
        for (size_t i = 0; i < groups.size(); ++i) {
            m_log->debug("group {}: {} hits", i, groups[i].size());
        }

        // form clusters
        for (const auto &group : groups) {
            if (static_cast<int>(group.size()) < m_cfg.minClusterNhits) {
                continue;
            }
            double energy = 0.;
            for (std::size_t idx : group) {
                energy += hits[idx].getEnergy();
            }
            if (energy < minClusterEdep) {
                continue;
            }
            auto pcl = proto->create();
            for (std::size_t idx : group) {
                pcl.addToHits(hits[idx]);
                pcl.addToWeights(1);
            }
        }

        return std::move(proto);
    }

  private:

    // helper function to group hits
    bool is_neighbour(const edm4eic::CalorimeterHit& h1, const edm4eic::CalorimeterHit& h2) const {
        // different sectors, simple distance check
        if (h1.getSector() != h2.getSector()) {
            return std::hypot((h1.getPosition().x - h2.getPosition().x),
                              (h1.getPosition().y - h2.getPosition().y),
                              (h1.getPosition().z - h2.getPosition().z)) <= sectorDist;
        }

        // layer check
        int ldiff = std::abs(h1.getLayer() - h2.getLayer());
        // same layer, check local positions
        if (ldiff == 0) {
            return (std::abs(h1.getLocal().x - h2.getLocal().x) <= localDistXY[0]) &&
                   (std::abs(h1.getLocal().y - h2.getLocal().y) <= localDistXY[1]);
        } else if (ldiff <= m_cfg.neighbourLayersRange) {
            return (std::abs(edm4eic::eta(h1.getPosition()) - edm4eic::eta(h2.getPosition())) <= layerDistEtaPhi[0]) &&
                   (std::abs(edm4eic::angleAzimuthal(h1.getPosition()) - edm4eic::angleAzimuthal(h2.getPosition())) <=
                    layerDistEtaPhi[1]);
        }

        // not in adjacent layers
        return false;
    }

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
  };

} // namespace eicrecon
