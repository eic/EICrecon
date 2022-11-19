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
#include "fmt/format.h"
#include <algorithm>

#include "DD4hep/BitFieldCoder.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/Surface.h"
#include "DDRec/SurfaceManager.h"

#include <spdlog/spdlog.h>

// Event Model related classes
#include "edm4eic/CalorimeterHit.h"
#include "edm4eic/ProtoCluster.h"
#include "edm4eic/vector_utils.h"


//namespace Jug::Reco {

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
class ImagingTopoCluster {
protected:

    // maximum difference in layer numbers that can be considered as neighbours
    int m_neighbourLayersRange = 1; // {this, "neighbourLayersRange", 1};
    // maximum distance of local (x, y) to be considered as neighbors at the same layer
    std::vector<double> u_localDistXY = {1.0 * dd4hep::mm, 1.0 * dd4hep::mm}; //{this, "localDistXY", {1.0 * dd4hep::mm, 1.0 * dd4hep::mm}};
    // maximum distance of global (eta, phi) to be considered as neighbors at different layers
    std::vector<double> u_layerDistEtaPhi = {0.01, 0.01}; //{this, "layerDistEtaPhi", {0.01, 0.01}};
    // maximum global distance to be considered as neighbors in different sectors
    double m_sectorDist = 1.0 * dd4hep::cm; // {this, "sectorDist", 1.0 * dd4hep::cm};

    // minimum hit energy to participate clustering
    double m_minClusterHitEdep = 0.; // {this, "minClusterHitEdep", 0.};
    // minimum cluster center energy (to be considered as a seed for cluster)
    double m_minClusterCenterEdep = 0.; // {this, "minClusterCenterEdep", 0.};
    // minimum cluster energy (to save this cluster)
    double m_minClusterEdep = 0.5 * dd4hep::MeV; // {this, "minClusterEdep", 0.5 * dd4hep::MeV};
    // minimum number of hits (to save this cluster)
    int m_minClusterNhits = 10; // {this, "minClusterNhits", 10};
    // input hits collection
    std::vector<const edm4eic::CalorimeterHit *> m_inputHits;
    // output clustered hits
    std::vector<edm4eic::ProtoCluster *> m_outputProtoClusters;

    std::shared_ptr<spdlog::logger> m_log;

    // unitless counterparts of the input parameters
    double localDistXY[2]{0, 0}, layerDistEtaPhi[2]{0, 0}, sectorDist{0};
    double minClusterHitEdep{0}, minClusterCenterEdep{0}, minClusterEdep{0}, minClusterNhits{0};

public:
    ImagingTopoCluster() = default;

    void initialize() {
        // unitless conversion
        // sanity checks
        if (u_localDistXY.size() != 2) {
            m_log->error( "Expected 2 values (x_dist, y_dist) for localDistXY");
            return;
        }
        if (u_layerDistEtaPhi.size() != 2) {
            m_log->error( "Expected 2 values (eta_dist, phi_dist) for layerDistEtaPhi" );
            return;
        }

        // using juggler internal units (GeV, dd4hep::mm, dd4hep::ns, dd4hep::rad)
        localDistXY[0] = u_localDistXY[0] / dd4hep::mm;
        localDistXY[1] = u_localDistXY[1] / dd4hep::mm;
        layerDistEtaPhi[0] = u_layerDistEtaPhi[0];
        layerDistEtaPhi[1] = u_layerDistEtaPhi[1] / dd4hep::rad;
        sectorDist = m_sectorDist / dd4hep::mm;
        minClusterHitEdep = m_minClusterHitEdep / dd4hep::GeV;
        minClusterCenterEdep = m_minClusterCenterEdep / dd4hep::GeV;
        minClusterEdep = m_minClusterEdep / dd4hep::GeV;

        // summarize the clustering parameters
        m_log->info( fmt::format("Local clustering (same sector and same layer): "
                              "Local [x, y] distance between hits <= [{:.4f} dd4hep::mm, {:.4f} dd4hep::mm].",
                              localDistXY[0], localDistXY[1])
        );
        m_log->info( fmt::format("Neighbour layers clustering (same sector and layer id within +- {:d}: "
                              "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} dd4hep::rad].",
                              m_neighbourLayersRange, layerDistEtaPhi[0], layerDistEtaPhi[1])
        );
        m_log->info( fmt::format("Neighbour sectors clustering (different sector): "
                              "Global distance between hits <= {:.4f} dd4hep::mm.",
                              sectorDist)
        );

        return;
    }

    void execute()  {
        // input collections
        auto &hits = m_inputHits;
        // Create output collections
        auto &proto = m_outputProtoClusters;

        // group neighboring hits
        std::vector<bool> visits(hits.size(), false);
        std::vector<std::vector<std::pair<uint32_t, const edm4eic::CalorimeterHit*>>> groups;
        for (size_t i = 0; i < hits.size(); ++i) {
            if(m_log->level() == SPDLOG_LEVEL_DEBUG) {
                m_log->debug(fmt::format("hit {:d}: local position = ({}, {}, {}), global position = ({}, {}, {})", i + 1,
                                       hits[i]->getLocal().x, hits[i]->getLocal().y, hits[i]->getPosition().z,
                                       hits[i]->getPosition().x, hits[i]->getPosition().y, hits[i]->getPosition().z)
                );
            }
            // already in a group, or not energetic enough to form a cluster
            if (visits[i] || hits[i]->getEnergy() < minClusterCenterEdep) {
                continue;
            }
            groups.emplace_back();
            // create a new group, and group all the neighboring hits
            dfs_group(groups.back(), i, hits, visits);
        }
        if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
            m_log->debug(fmt::format("found {} potential clusters (groups of hits)", groups.size() ));
            for (size_t i = 0; i < groups.size(); ++i) {
                m_log->debug( fmt::format("group {}: {} hits", i, groups[i].size()) );
            }
        }

        // form clusters
        for (const auto &group: groups) {
            if (static_cast<int>(group.size()) < m_minClusterNhits) {
                continue;
            }
            double energy = 0.;
            for (const auto &[idx, hit]: group) {
                energy += hit->getEnergy();
            }
            if (energy < minClusterEdep) {
                continue;
            }
            edm4eic::MutableProtoCluster pcl;
            for (const auto &[idx, hit]: group) {
                pcl.addToHits(*hit);
                pcl.addToWeights(1);
            }
            proto.push_back( new edm4eic::ProtoCluster(pcl) );
        }

        return;
    }

private:
    template<typename T> static inline T pow2(const T &x) { return x * x; }

    // helper function to group hits
    bool is_neighbor(const edm4eic::CalorimeterHit *h1, const edm4eic::CalorimeterHit *h2) const {
        // different sectors, simple distance check
        if (h1->getSector() != h2->getSector()) {
            return std::sqrt(
                    pow2(h1->getPosition().x - h2->getPosition().x) + pow2(h1->getPosition().y - h2->getPosition().y) +
                    pow2(h1->getPosition().z - h2->getPosition().z)) <= sectorDist;
        }

        // layer check
        int ldiff = std::abs(h1->getLayer() - h2->getLayer());
        // same layer, check local positions
        if (ldiff == 0) {
            return (std::abs(h1->getLocal().x - h2->getLocal().x) <= localDistXY[0]) &&
                   (std::abs(h1->getLocal().y - h2->getLocal().y) <= localDistXY[1]);
        } else if (ldiff <= m_neighbourLayersRange) {
            return (std::abs(edm4eic::eta(h1->getPosition()) - edm4eic::eta(h2->getPosition())) <= layerDistEtaPhi[0]) &&
                   (std::abs(edm4eic::angleAzimuthal(h1->getPosition()) - edm4eic::angleAzimuthal(h2->getPosition())) <=
                    layerDistEtaPhi[1]);
        }

        // not in adjacent layers
        return false;
    }

    // grouping function with Depth-First Search
    void dfs_group(std::vector<std::pair<uint32_t, const edm4eic::CalorimeterHit*>> &group, int idx,
                   std::vector<const edm4eic::CalorimeterHit*> &hits, std::vector<bool> &visits) const {
        // not a qualified hit to participate in clustering, stop here
        if (hits[idx]->getEnergy() < minClusterHitEdep) {
            visits[idx] = true;
            return;
        }

        group.emplace_back(idx, hits[idx]);
        visits[idx] = true;
        for (size_t i = 0; i < hits.size(); ++i) {
            // visited, or not a neighbor
            if (visits[i] || !is_neighbor(hits[idx], hits[i])) {
                continue;
            }
            dfs_group(group, i, hits, visits);
        }
    }
}; // namespace Jug::Reco

//} // namespace Jug::Reco
