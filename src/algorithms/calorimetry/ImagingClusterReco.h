// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao Peng, Wouter Deconinck, David Lawrence

/*
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 */

#pragma once

#include <Eigen/Dense>
#include <algorithm>

#include <algorithms/algorithm.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>

#include "algorithms/calorimetry/ClusterTypes.h"

// Event Model related classes
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "ImagingClusterRecoConfig.h"

namespace eicrecon {

  using ImagingClusterRecoAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ProtoClusterCollection,
      edm4hep::SimCalorimeterHitCollection
    >,
    algorithms::Output<
      edm4eic::ClusterCollection,
      edm4eic::MCRecoClusterParticleAssociationCollection,
      edm4eic::ClusterCollection
    >
  >;

  /** Imaging cluster reconstruction.
   *
   *  Reconstruct the cluster/layer info for imaging calorimeter
   *  Logarithmic weighting is used to describe energy deposit in transverse direction
   *
   *  \ingroup reco
   */
  class ImagingClusterReco
      : public ImagingClusterRecoAlgorithm,
        public WithPodConfig<ImagingClusterRecoConfig> {

  public:
    ImagingClusterReco(std::string_view name)
      : ImagingClusterRecoAlgorithm{name,
                            {"inputProtoClusterCollection", "mcHits"},
                            {"outputClusterCollection", "outputClusterAssociations", "outputLayerCollection"},
                            "Reconstruct the cluster/layer info for imaging calorimeter."} {}

  public:

    void init()  { }

    void process(const Input& input, const Output& output) const final {

        const auto [proto, mchits] = input;
        auto [clusters, associations, layers] = output;

        for (const auto& pcl: *proto) {
            if (!pcl.getHits().empty() && !pcl.getHits(0).isAvailable()) {
                warning("Protocluster hit relation is invalid, skipping protocluster");
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
            for (const auto& layer: cl_layers) {
                layers->push_back(layer);
                cl.addToClusters(layer);
            }
            clusters->push_back(cl);

            // If mcHits are available, associate cluster with MCParticle
            if (mchits->size() > 0) {

                // 1. find pclhit with the largest energy deposition
                auto pclhits = pcl.getHits();
                auto pclhit = std::max_element(
                        pclhits.begin(),
                        pclhits.end(),
                        [](const auto &pclhit1, const auto &pclhit2) {
                            return pclhit1.getEnergy() < pclhit2.getEnergy();
                        }
                );

                // 2. find mchit with same CellID
                const edm4hep::SimCalorimeterHit* mchit = nullptr;
                for (auto h : *mchits) {
                    if (h.getCellID() == pclhit->getCellID()) {
                        mchit = &h;
                        break;
                    }
                }
                if( !mchit ){
                    // break if no matching hit found for this CellID
                    warning("Proto-cluster has highest energy in CellID {}, but no mc hit with that CellID was found.", pclhit->getCellID());
                    break;
                }

                // 3. find mchit's MCParticle
                const auto &mcp = mchit->getContributions(0).getParticle();

                // set association
                auto clusterassoc = associations->create();
                clusterassoc.setRecID(cl.getObjectID().index);
                clusterassoc.setSimID(mcp.getObjectID().index);
                clusterassoc.setWeight(1.0);
                clusterassoc.setRec(cl);
                clusterassoc.setSim(mcp);
            }

        }

        // debug output
        for (const auto& cl: *clusters) {
            debug("Cluster {:d}: Edep = {:.3f} MeV, Dir = ({:.3f}, {:.3f}) deg", cl.getObjectID().index,
                         cl.getEnergy() * 1000., cl.getIntrinsicTheta() / M_PI * 180.,
                         cl.getIntrinsicPhi() / M_PI * 180.
            );
        }
    }

  private:

    static std::vector<edm4eic::MutableCluster> reconstruct_cluster_layers(const edm4eic::ProtoCluster& pcl) {
        const auto& hits = pcl.getHits();
        const auto& weights = pcl.getWeights();
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
        std::vector<edm4eic::MutableCluster> cl_layers;
        for (const auto &[lid, layer_hits]: layer_map) {
            auto layer = reconstruct_layer(layer_hits);
            cl_layers.push_back(layer);
        }
        return cl_layers;
    }

    static edm4eic::MutableCluster reconstruct_layer(const std::vector<std::pair<const edm4eic::CalorimeterHit, float>>& hits) {
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
            radius += std::pow(edm4hep::utils::magnitude(hit.getPosition() - layer.getPosition()), 2);
        }
        layer.addToShapeParameters(std::sqrt(radius / layer.getNhits()));
        // TODO Skewedness

        return layer;
    }

    static edm4eic::MutableCluster reconstruct_cluster(const edm4eic::ProtoCluster& pcl) {
        edm4eic::MutableCluster cluster;

        const auto& hits = pcl.getHits();
        const auto& weights = pcl.getWeights();

        cluster.setType(Jug::Reco::ClusterType::kCluster3D);
        double energy = 0.;
        double energyError = 0.;
        double time = 0.;
        double timeError = 0.;
        double meta = 0.;
        double mphi = 0.;
        double r = 9999 * dd4hep::cm;
        for (unsigned i = 0; i < hits.size(); ++i) {
            const auto &hit = hits[i];
            const auto &weight = weights[i];
            energy += hit.getEnergy() * weight;
            energyError += std::pow(hit.getEnergyError() * weight, 2);
            // energy weighting for the other variables
            const double energyWeight = hit.getEnergy() * weight;
            time += hit.getTime() * energyWeight;
            timeError += std::pow(hit.getTimeError() * energyWeight, 2);
            meta += edm4hep::utils::eta(hit.getPosition()) * energyWeight;
            mphi += edm4hep::utils::angleAzimuthal(hit.getPosition()) * energyWeight;
            r = std::min(edm4hep::utils::magnitude(hit.getPosition()), r);
            cluster.addToHits(hit);
        }
        cluster.setEnergy(energy);
        cluster.setEnergyError(std::sqrt(energyError));
        cluster.setTime(time / energy);
        cluster.setTimeError(std::sqrt(timeError) / energy);
        cluster.setNhits(hits.size());
        cluster.setPosition(edm4hep::utils::sphericalToVector(r, edm4hep::utils::etaToAngle(meta / energy), mphi / energy));

        // shower radius estimate (eta-phi plane)
        double radius = 0.;
        for (const auto &hit: hits) {
            radius += std::pow(
              std::hypot(
                (edm4hep::utils::eta(hit.getPosition()) - edm4hep::utils::eta(cluster.getPosition())),
                (edm4hep::utils::angleAzimuthal(hit.getPosition()) - edm4hep::utils::angleAzimuthal(cluster.getPosition()))
              ),
              2.0
            );
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

    std::pair<double /* polar */, double /* azimuthal */> fit_track(const std::vector<edm4eic::MutableCluster> &layers) const {
        int nrows = 0;
        decltype(edm4eic::ClusterData::position) mean_pos{0, 0, 0};
        for (const auto &layer: layers) {
            if ((layer.getNhits() > 0) && (layer.getHits(0).getLayer() <= m_cfg.trackStopLayer)) {
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
            if ((layer.getNhits() > 0) && (layer.getHits(0).getLayer() <= m_cfg.trackStopLayer)) {
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

} // namespace eicrecon
