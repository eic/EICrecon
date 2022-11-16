// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao Peng, Wouter Deconinck, David Lawrence

/*
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 */
#include "fmt/format.h"
#include <Eigen/Dense>
#include <algorithm>

#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/Surface.h"
#include "DDRec/SurfaceManager.h"

#include <algorithms/calorimetry/ClusterTypes.h>

//
//#include "JugBase/DataHandle.h"
//#include "JugBase/IGeoSvc.h"
//#include "JugBase/Utilities/Utils.hpp"
//#include "JugReco/ClusterTypes.h"

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4eic/CalorimeterHitCollection.h"
#include "edm4eic/ClusterCollection.h"
#include "edm4eic/MCRecoClusterParticleAssociationCollection.h"
#include "edm4eic/ProtoClusterCollection.h"
#include "edm4eic/vector_utils.h"

//using namespace Gaudi::Units;
//using namespace Eigen;

//namespace Jug::Reco {

/** Imaging cluster reconstruction.
 *
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  \ingroup reco
 */
class ImagingClusterReco {
protected:
    int m_trackStopLayer = 9; // {this, "trackStopLayer", 9};

    std::vector<const edm4eic::ProtoCluster *> m_inputProtoClusters;
    std::vector<edm4eic::Cluster *> m_outputLayers;
    std::vector<edm4eic::Cluster *> m_outputClusters;

    // Collection for MC hits when running on MC
    std::string m_mcHits_name; // {this, "mcHits", ""};
    // Optional handle to MC hits
    std::vector<const edm4hep::SimCalorimeterHit *> m_mcHits;

    // Collection for associations when running on MC
    std::string m_outputAssociations_name; // {this, "outputAssociations", ""};
    // Optional handle to MC hits
    std::vector<edm4eic::MCRecoClusterParticleAssociation *> m_outputAssociations;

    // logger
    std::shared_ptr<spdlog::logger> m_log;

public:
    ImagingClusterReco() = default;

    void initialize() {}

    void execute() {
        // input collections
        const auto &proto = m_inputProtoClusters;
        // output collections
        auto &layers = m_outputLayers;
        auto &clusters = m_outputClusters;

        for (auto pcl: proto) {
            if (!pcl->getHits().empty() && !pcl->getHits(0).isAvailable()) {
                m_log->warn("Protocluster hit relation is invalid, skipping protocluster");
                continue;
            }
            // get cluster and associated layers
            auto cl = reconstruct_cluster(pcl);
            auto cl_layers = reconstruct_cluster_layers(pcl);

            // Get cluster direction from the layer profile
            auto [theta, phi] = fit_track(cl_layers);
            cl.setIntrinsicTheta(theta);
            cl.setIntrinsicPhi(phi);
            // no error on the intrinsic direction TODO

            // store layer and clusters on the datastore
            for (const auto &layer: cl_layers) {
                auto layer_ptr = new edm4eic::Cluster( layer );
                layers.push_back(layer_ptr);
//                cl.addToClusters(*layer_ptr);
            }
            clusters.push_back(new edm4eic::Cluster( cl ));

            // If mcHits are available, associate cluster with MCParticle
            if ( (!m_mcHits.empty()) && (!m_outputAssociations.empty()) ) {

                // 1. find pclhit with largest energy deposition
                auto pclhits = pcl->getHits();
                auto pclhit = std::max_element(
                        pclhits.begin(),
                        pclhits.end(),
                        [](const auto &pclhit1, const auto &pclhit2) {
                            return pclhit1.getEnergy() < pclhit2.getEnergy();
                        }
                );

                // 2. find mchit with same CellID
                const edm4hep::SimCalorimeterHit* mchit = nullptr;
                for( auto h : m_mcHits ){
                    if (h->getCellID() == pclhit->getCellID()) {
                        mchit = h;
                        break;
                    }
                }
                if( !mchit ){
                    // break if no matching hit found for this CellID
                    m_log->warn( fmt::format("Proto-cluster has highest energy in CellID {}, but no mc hit with that CellID was found.", pclhit->getCellID() ));
                    break;
                }

                // 3. find mchit's MCParticle
                const auto &mcp = mchit->getContributions(0).getParticle();

                // set association
                edm4eic::MutableMCRecoClusterParticleAssociation clusterassoc;
                clusterassoc.setRecID(cl.getObjectID().index);
                clusterassoc.setSimID(mcp.getObjectID().index);
                clusterassoc.setWeight(1.0);
                clusterassoc.setRec(cl);
                //clusterassoc.setSim(mcp);
                m_outputAssociations.push_back(new edm4eic::MCRecoClusterParticleAssociation(clusterassoc));
            }

        }

        // debug output
        if (m_log->level() == SPDLOG_LEVEL_DEBUG) {
            for (const auto &cl: clusters) {
                m_log->debug( fmt::format("Cluster {:d}: Edep = {:.3f} MeV, Dir = ({:.3f}, {:.3f}) deg", cl->id(),
                                       cl->getEnergy() * 1000., cl->getIntrinsicTheta() / M_PI * 180.,
                                       cl->getIntrinsicPhi() / M_PI * 180.)
                );
            }
        }
    }

private:
    template<typename T>
    static inline T pow2(const T &x) { return x * x; }

    static std::vector<edm4eic::Cluster> reconstruct_cluster_layers(const edm4eic::ProtoCluster *pcl) {
        const auto &hits = pcl->getHits();
        const auto &weights = pcl->getWeights();
        // using map to have hits sorted by layer
        std::map<int, std::vector<std::pair<const edm4eic::CalorimeterHit, float>>> layer_map;
        for (unsigned i = 0; i < hits.size(); ++i) {
            const auto hit = hits[i];
            auto lid = hit.getLayer();
//            if (layer_map.count(lid) == 0) {
//                std::vector<std::pair<const edm4eic::CalorimeterHit, float>> v;
//                layer_map[lid] = {};
//            }
            layer_map[lid].push_back({hit, weights[i]});
        }

        // create layers
        std::vector<edm4eic::Cluster> cl_layers;
        for (const auto &[lid, layer_hits]: layer_map) {
            auto layer = reconstruct_layer(layer_hits);
            cl_layers.push_back(layer);
        }
        return cl_layers;
    }

    static edm4eic::Cluster reconstruct_layer(const std::vector<std::pair<const edm4eic::CalorimeterHit, float>> &hits) {
        edm4eic::MutableCluster layer;
        layer.setType(Jug::Reco::ClusterType::kClusterSlice);
        // Calculate averages
        double energy{0};
        double energyError{0};
        double time{0};
        double timeError{0};
        double sumOfWeights{0};
        auto pos = layer.getPosition();
        for (const auto &[hit, weight]: hits) {
            energy += hit.getEnergy() * weight;
            energyError += std::pow(hit.getEnergyError() * weight, 2);
            time += hit.getTime() * weight;
            timeError += std::pow(hit.getTimeError() * weight, 2);
            pos = pos + hit.getPosition() * weight;
            sumOfWeights += weight;
            layer.addToHits(hit);
        }
        layer.setEnergy(energy);
        layer.setEnergyError(std::sqrt(energyError));
        layer.setTime(time / sumOfWeights);
        layer.setTimeError(std::sqrt(timeError) / sumOfWeights);
        layer.setNhits(hits.size());
        layer.setPosition(pos / sumOfWeights);
        // positionError not set
        // Intrinsic direction meaningless in a cluster layer --> not set

        // Calculate radius as the standard deviation of the hits versus the cluster center
        double radius = 0.;
        for (const auto &[hit, weight]: hits) {
            radius += std::pow(edm4eic::magnitude(hit.getPosition() - layer.getPosition()), 2);
        }
        layer.addToShapeParameters(std::sqrt(radius / layer.getNhits()));
        // TODO Skewedness

        return layer;
    }

    edm4eic::MutableCluster reconstruct_cluster(const edm4eic::ProtoCluster *pcl) {
        edm4eic::MutableCluster cluster;

        const auto &hits = pcl->getHits();
        const auto &weights = pcl->getWeights();

        cluster.setType(Jug::Reco::ClusterType::kCluster3D);
        double energy = 0.;
        double energyError = 0.;
        double time = 0.;
        double timeError = 0.;
        double meta = 0.;
        double mphi = 0.;
        double r = 9999 * cm;
        for (unsigned i = 0; i < hits.size(); ++i) {
            const auto &hit = hits[i];
            const auto &weight = weights[i];
            energy += hit.getEnergy() * weight;
            energyError += std::pow(hit.getEnergyError() * weight, 2);
            // energy weighting for the other variables
            const double energyWeight = hit.getEnergy() * weight;
            time += hit.getTime() * energyWeight;
            timeError += std::pow(hit.getTimeError() * energyWeight, 2);
            meta += edm4eic::eta(hit.getPosition()) * energyWeight;
            mphi += edm4eic::angleAzimuthal(hit.getPosition()) * energyWeight;
            r = std::min(edm4eic::magnitude(hit.getPosition()), r);
//            cluster.addToHits(hit);
        }
        cluster.setEnergy(energy);
        cluster.setEnergyError(std::sqrt(energyError));
        cluster.setTime(time / energy);
        cluster.setTimeError(std::sqrt(timeError) / energy);
        cluster.setNhits(hits.size());
        cluster.setPosition(edm4eic::sphericalToVector(r, edm4eic::etaToAngle(meta / energy), mphi / energy));

        // shower radius estimate (eta-phi plane)
        double radius = 0.;
        for (const auto &hit: hits) {
            radius += pow2(edm4eic::eta(hit.getPosition()) - edm4eic::eta(cluster.getPosition())) +
                      pow2(edm4eic::angleAzimuthal(hit.getPosition()) - edm4eic::angleAzimuthal(cluster.getPosition()));
        }
        cluster.addToShapeParameters(std::sqrt(radius / cluster.getNhits()));
        // Skewedness not calculated TODO

        // Optionally store the MC truth associated with the first hit in this cluster
        // FIXME no connection between cluster and truth in edm4hep
        // if (mcHits) {
        //  const auto& mc_hit    = (*mcHits)[pcl.getHits(0).ID.value];
        //  cluster.mcID({mc_hit.truth().trackID, m_kMonteCarloSource});
        //}

        return cluster;
    }

    std::pair<double /* polar */, double /* azimuthal */> fit_track(const std::vector<edm4eic::Cluster> &layers) const {
        int nrows = 0;
        decltype(edm4eic::ClusterData::position) mean_pos{0, 0, 0};
        for (const auto &layer: layers) {
            if ((layer.getNhits() > 0) && (layer.getHits(0).getLayer() <= m_trackStopLayer)) {
                mean_pos = mean_pos + layer.getPosition();
                nrows += 1;
            }
        }
        // cannot fit
        if (nrows < 2) {
            return {};
        }

        mean_pos = mean_pos / nrows;
        // fill position data
        Eigen::MatrixXd pos(nrows, 3);
        int ir = 0;
        for (const auto &layer: layers) {
            if ((layer.getNhits() > 0) && (layer.getHits(0).getLayer() <= m_trackStopLayer)) {
                auto delta = layer.getPosition() - mean_pos;
                pos(ir, 0) = delta.x;
                pos(ir, 1) = delta.y;
                pos(ir, 2) = delta.z;
                ir += 1;
            }
        }

        Eigen::JacobiSVD <Eigen::MatrixXd> svd(pos, Eigen::ComputeThinU | Eigen::ComputeThinV);
        const auto dir = svd.matrixV().col(0);
        // theta and phi
        return {std::acos(dir(2)), std::atan2(dir(1), dir(0))};
    }
};


//} // namespace Jug::Reco
