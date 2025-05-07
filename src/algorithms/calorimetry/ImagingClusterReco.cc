// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Chao Peng, Wouter Deconinck, David Lawrence, Derek Anderson

/*
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 */

#include "algorithms/calorimetry/ImagingClusterReco.h"

#include "algorithms/calorimetry/ImagingClusterRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

void ImagingClusterReco::process(const Input& input, const Output& output) const {

#if EDM4EIC_VERSION_MAJOR >= 7
  const auto [proto, mchitassociations] = input;
#else
  const auto [proto, mchits] = input;
#endif
  auto [clusters, associations, layers] = output;

  for (const auto& pcl : *proto) {
    if (!pcl.getHits().empty() && !pcl.getHits(0).isAvailable()) {
      warning("Protocluster hit relation is invalid, skipping protocluster");
      continue;
    }
    // get cluster and associated layers
    auto cl        = reconstruct_cluster(pcl);
    auto cl_layers = reconstruct_cluster_layers(pcl);

    // Get cluster direction from the layer profile
    auto [theta, phi] = fit_track(cl_layers);
    cl.setIntrinsicTheta(theta);
    cl.setIntrinsicPhi(phi);
    // no error on the intrinsic direction TODO

    // store layer and clusters on the datastore
    for (const auto& layer : cl_layers) {
      layers->push_back(layer);
      cl.addToClusters(layer);
    }
    clusters->push_back(cl);

    // If sim hits are available, associate cluster with MCParticle
#if EDM4EIC_VERSION_MAJOR >= 7
    if (mchitassociations->size() == 0) {
      debug("Provided MCRecoCalorimeterHitAssociation collection is empty. No truth associations "
            "will be performed.");
      continue;
    } else {
      associate_mc_particles(cl, mchitassociations, associations);
    }
#else
    if (mchits->size() == 0) {
      debug("Provided SimCalorimeterHitCollection is empty. No truth association will be "
            "performed.");
      continue;
    } else {
      associate_mc_particles(cl, mchits, associations);
    }
#endif
  }

  // debug output
  for (const auto& cl : *clusters) {
    debug("Cluster {:d}: Edep = {:.3f} MeV, Dir = ({:.3f}, {:.3f}) deg", cl.getObjectID().index,
          cl.getEnergy() * 1000., cl.getIntrinsicTheta() / M_PI * 180.,
          cl.getIntrinsicPhi() / M_PI * 180.);
  }
}

std::vector<edm4eic::MutableCluster>
ImagingClusterReco::reconstruct_cluster_layers(const edm4eic::ProtoCluster& pcl) {
  const auto& hits    = pcl.getHits();
  const auto& weights = pcl.getWeights();
  // using map to have hits sorted by layer
  std::map<int, std::vector<std::pair<const edm4eic::CalorimeterHit, float>>> layer_map;
  for (unsigned i = 0; i < hits.size(); ++i) {
    const auto hit = hits[i];
    auto lid       = hit.getLayer();
    //            if (layer_map.count(lid) == 0) {
    //                std::vector<std::pair<const edm4eic::CalorimeterHit, float>> v;
    //                layer_map[lid] = {};
    //            }
    layer_map[lid].push_back({hit, weights[i]});
  }

  // create layers
  std::vector<edm4eic::MutableCluster> cl_layers;
  for (const auto& [lid, layer_hits] : layer_map) {
    auto layer = reconstruct_layer(layer_hits);
    cl_layers.push_back(layer);
  }
  return cl_layers;
}

edm4eic::MutableCluster ImagingClusterReco::reconstruct_layer(
    const std::vector<std::pair<const edm4eic::CalorimeterHit, float>>& hits) {
  edm4eic::MutableCluster layer;
  layer.setType(Jug::Reco::ClusterType::kClusterSlice);
  // Calculate averages
  double energy{0};
  double energyError{0};
  double time{0};
  double timeError{0};
  double sumOfWeights{0};
  auto pos = layer.getPosition();
  for (const auto& [hit, weight] : hits) {
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
  for (const auto& [hit, weight] : hits) {
    radius += std::pow(edm4hep::utils::magnitude(hit.getPosition() - layer.getPosition()), 2);
  }
  layer.addToShapeParameters(std::sqrt(radius / layer.getNhits()));
  // TODO Skewedness

  return layer;
}

edm4eic::MutableCluster ImagingClusterReco::reconstruct_cluster(const edm4eic::ProtoCluster& pcl) {
  edm4eic::MutableCluster cluster;

  const auto& hits    = pcl.getHits();
  const auto& weights = pcl.getWeights();

  cluster.setType(Jug::Reco::ClusterType::kCluster3D);
  double energy      = 0.;
  double energyError = 0.;
  double time        = 0.;
  double timeError   = 0.;
  double meta        = 0.;
  double mphi        = 0.;
  double r           = 9999 * dd4hep::cm;
  for (unsigned i = 0; i < hits.size(); ++i) {
    const auto& hit    = hits[i];
    const auto& weight = weights[i];
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
  cluster.setPosition(edm4hep::utils::sphericalToVector(
      r, edm4hep::utils::etaToAngle(meta / energy), mphi / energy));

  // shower radius estimate (eta-phi plane)
  double radius = 0.;
  for (const auto& hit : hits) {
    radius += std::pow(std::hypot((edm4hep::utils::eta(hit.getPosition()) -
                                   edm4hep::utils::eta(cluster.getPosition())),
                                  (edm4hep::utils::angleAzimuthal(hit.getPosition()) -
                                   edm4hep::utils::angleAzimuthal(cluster.getPosition()))),
                       2.0);
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

std::pair<double /* polar */, double /* azimuthal */>
ImagingClusterReco::fit_track(const std::vector<edm4eic::MutableCluster>& layers) const {
  int nrows = 0;
  decltype(edm4eic::ClusterData::position) mean_pos{0, 0, 0};
  for (const auto& layer : layers) {
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
  for (const auto& layer : layers) {
    if ((layer.getNhits() > 0) && (layer.getHits(0).getLayer() <= m_cfg.trackStopLayer)) {
      auto delta = layer.getPosition() - mean_pos;
      pos(ir, 0) = delta.x;
      pos(ir, 1) = delta.y;
      pos(ir, 2) = delta.z;
      ir += 1;
    }
  }

  Eigen::JacobiSVD<Eigen::MatrixXd> svd(pos, Eigen::ComputeThinU | Eigen::ComputeThinV);
  const auto dir = svd.matrixV().col(0);
  // theta and phi
  return {std::acos(dir(2)), std::atan2(dir(1), dir(0))};
}

void ImagingClusterReco::associate_mc_particles(
    const edm4eic::Cluster& cl,
#if EDM4EIC_VERSION_MAJOR >= 7
    const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
#else
    const edm4hep::SimCalorimeterHitCollection* mchits,
#endif
    edm4eic::MCRecoClusterParticleAssociationCollection* assocs) const {
  // --------------------------------------------------------------------------
  // Association Logic
  // --------------------------------------------------------------------------
  /*  1. identify all sim hits associated with a given protocluster, and sum
         *     the energy of the sim hits.
         *  2. for each sim hit
         *     - identify parents of each contributing particles; and
         *     - if parent is a primary particle, add to list of contributors
         *       and sum the energy contributed by the parent.
         *  3. create an association for each contributing primary with a weight
         *     of contributed energy over total sim hit energy.
         */

  // lambda to compare MCParticles
  auto compare = [](const edm4hep::MCParticle& lhs, const edm4hep::MCParticle& rhs) {
    if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
      return (lhs.getObjectID().index < rhs.getObjectID().index);
    } else {
      return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
    }
  };

  // bookkeeping maps for associated primaries
  std::map<edm4hep::MCParticle, double, decltype(compare)> mapMCParToContrib(compare);

  // --------------------------------------------------------------------------
  // 1. get associated sim hits and sum energy
  // --------------------------------------------------------------------------
  double eSimHitSum = 0.;
  for (auto clhit : cl.getHits()) {
    // vector to hold associated sim hits
    std::vector<edm4hep::SimCalorimeterHit> vecAssocSimHits;

#if EDM4EIC_VERSION_MAJOR >= 7
    for (const auto& hitAssoc : *mchitassociations) {
      // if found corresponding raw hit, add sim hit to vector
      // and increment energy sum
      if (clhit.getRawHit() == hitAssoc.getRawHit()) {
        vecAssocSimHits.push_back(hitAssoc.getSimHit());
        eSimHitSum += vecAssocSimHits.back().getEnergy();
      }
    }
#else
    for (const auto& mchit : *mchits) {
      if (mchit.getCellID() == clhit.getCellID()) {
        vecAssocSimHits.push_back(mchit);
        eSimHitSum += vecAssocSimHits.back().getEnergy();
        break;
      }
    }

    // if no matching cell ID found, continue
    // otherwise increment sum
    if (vecAssocSimHits.empty()) {
      debug("No matching SimHit for hit {}", clhit.getCellID());
      continue;
    }
#endif
    debug("{} associated sim hits found for reco hit (cell ID = {})", vecAssocSimHits.size(),
          clhit.getCellID());

    // ------------------------------------------------------------------------
    // 2. loop through associated sim hits
    // ------------------------------------------------------------------------
    for (const auto& simHit : vecAssocSimHits) {
      for (const auto& contrib : simHit.getContributions()) {
        // --------------------------------------------------------------------
        // grab primary responsible for contribution & increment relevant sum
        // --------------------------------------------------------------------
        edm4hep::MCParticle primary = get_primary(contrib);
        mapMCParToContrib[primary] += contrib.getEnergy();

        trace("Identified primary: id = {}, pid = {}, total energy = {}, contributed = {}",
              primary.getObjectID().index, primary.getPDG(), primary.getEnergy(),
              mapMCParToContrib[primary]);
      }
    }
  }
  debug("Found {} primaries contributing a total of {} GeV", mapMCParToContrib.size(), eSimHitSum);

  // --------------------------------------------------------------------------
  // 3. create association for each contributing primary
  // --------------------------------------------------------------------------
  for (auto [part, contribution] : mapMCParToContrib) {
    // calculate weight
    const double weight = contribution / eSimHitSum;

    // set association
    auto assoc = assocs->create();
    assoc.setRecID(cl.getObjectID().index); // if not using collection, this is always set to -1
    assoc.setSimID(part.getObjectID().index);
    assoc.setWeight(weight);
    assoc.setRec(cl);
    assoc.setSim(part);
    debug("Associated cluster #{} to MC Particle #{} (pid = {}, status = {}, energy = {}) with "
          "weight ({})",
          cl.getObjectID().index, part.getObjectID().index, part.getPDG(),
          part.getGeneratorStatus(), part.getEnergy(), weight);
  }
}

edm4hep::MCParticle
ImagingClusterReco::get_primary(const edm4hep::CaloHitContribution& contrib) const {
  // get contributing particle
  const auto contributor = contrib.getParticle();

  // walk back through parents to find primary
  //   - TODO finalize primary selection. This
  //     can be improved!!
  edm4hep::MCParticle primary = contributor;
  while (primary.parents_size() > 0) {
    if (primary.getGeneratorStatus() != 0)
      break;
    primary = primary.getParents(0);
  }
  return primary;
}

} // namespace eicrecon
