// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Chao Peng, Wouter Deconinck, David Lawrence, Derek Anderson

/*
 *  Reconstruct the cluster/layer info for imaging calorimeter
 *  Logarithmic weighting is used to describe energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 */

#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <gsl/pointers>
#include <podio/LinkNavigator.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <Eigen/Householder> // IWYU pragma: keep
#include <Eigen/Jacobi>
#include <Eigen/SVD>
#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include <memory>
#include <new>
#include <tuple>

#include "algorithms/calorimetry/ClusterTypes.h"
#include "algorithms/calorimetry/ImagingClusterReco.h"
#include "algorithms/calorimetry/ImagingClusterRecoConfig.h"
#include "algorithms/interfaces/CompareObjectID.h"
#include "algorithms/interfaces/LinkTruthUtils.h"

namespace eicrecon {

void ImagingClusterReco::process(const Input& input, const Output& output) const {

  const auto [proto, mchitlinks, mchitassociations] = input;
  auto [clusters, links, associations, layers]      = output;

  // Check if truth associations are possible
  const truth::EventLinkNavigator<edm4eic::MCRecoCalorimeterHitLinkCollection> link_nav(mchitlinks);
  const bool do_assoc = link_nav.enabled();
  if (!do_assoc) {
    debug("Provided MCRecoCalorimeterHitLink collection is empty. No truth associations "
          "will be performed.");
  }

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
    if (do_assoc) {
      associate_mc_particles(cl, mchitassociations, link_nav, links, associations);
    }
  }

  // debug output
  for (const auto& cl : *clusters) {
    debug("Cluster {:d}: Edep = {:.3f} MeV, Dir = ({:.3f}, {:.3f}) deg", cl.getObjectID().index,
          cl.getEnergy() * 1000., cl.getIntrinsicTheta() / M_PI * 180.,
          cl.getIntrinsicPhi() / M_PI * 180.);
  }
}

std::vector<edm4eic::MutableCluster>
ImagingClusterReco::reconstruct_cluster_layers(const edm4eic::ProtoCluster& pcl) const {
  const auto& hits    = pcl.getHits();
  const auto& weights = pcl.getWeights();
  // using map to have hits sorted by layer
  std::map<int, std::vector<std::pair<const edm4eic::CalorimeterHit, float>>> layer_map;
  for (unsigned i = 0; i < hits.size(); ++i) {
    const auto hit = hits[i];
    auto lid       = hit.getLayer();
    layer_map[lid].emplace_back(hit, weights[i]);
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
    const std::vector<std::pair<const edm4eic::CalorimeterHit, float>>& hits) const {
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

  // Shape parameters are calculated separately by CalorimeterClusterShape algorithm

  return layer;
}

template <typename HitRange>
edm4eic::MutableCluster ImagingClusterReco::estimate_position(
    const HitRange& hits,
    const PositionEstimatorConfig& est) const {
  // Returns a throwaway MutableCluster carrying only position + positionError,
  // computed according to the given recipe: average the top hits by energy,
  // within maxLayersForPos. With the default recipe (maxLayersForPos ~ unlimited,
  // truncatedMean, truncateFrac = 1.0), this reduces to a plain energy-weighted
  // mean over all hits in the cluster. Not stored on the datastore.
  edm4eic::MutableCluster result;

  // sort hits by layer then energy: prefer smaller layer, then larger energy
  //                     layer     energy                r, phi, eta
  typedef std::pair<std::pair<int, double>, std::array<double, 3>> AngInfo;
  auto cmp = [](const AngInfo& a, const AngInfo& b) {
    int alayer = a.first.first;
    int blayer = b.first.first;
    if (alayer != blayer) return alayer > blayer; // larger layer is less preferred
    double ae = a.first.second;
    double be = b.first.second;
    return ae < be;
  };

  int numHitsInLayers = 0;
  for (const auto& hit : hits) {
    if (hit.getLayer() <= est.maxLayersForPos)
      ++numHitsInLayers;
  }
  std::priority_queue<AngInfo, std::vector<AngInfo>, decltype(cmp)> pq(cmp);

  // Determine how many highest-energy hits to average for the position estimate.
  // Mode is selected explicitly via averagingMode:
  //   - fixedCount: average exactly numHitsForPos hits, or fewer if the
  //     cluster does not have that many hits within maxLayersForPos.
  //   - truncatedMean: average only the top fraction (truncateFrac) of hits
  //     by energy, rounded down, with a minimum of 1 hit.
  //     E.g. truncateFrac = 0.2 keeps the top 20% of hits; truncateFrac = 1.0
  //     (the default) keeps all of them, reducing to a plain mean.
  int numAve;
  if (est.averagingMode == PositionEstimatorConfig::EAveragingMode::fixedCount) {
    numAve = std::min(numHitsInLayers, est.numHitsForPos);
  } else {
    numAve = std::max(1, static_cast<int>(est.truncateFrac * numHitsInLayers));
  }

  // min-heap for top numAve hits
  for (const auto& hit : hits) {
    if (hit.getLayer() <= est.maxLayersForPos) {
      double E = hit.getEnergy();
      AngInfo info{{hit.getLayer(), E}, {edm4hep::utils::magnitude(hit.getPosition()),
                                         edm4hep::utils::angleAzimuthal(hit.getPosition()),
                                         edm4hep::utils::eta(hit.getPosition())}};
      if (pq.size() < static_cast<size_t>(numAve))
        pq.push(info);
      else if (cmp(pq.top(), info)) {
        pq.pop();
        pq.push(info);
      }
    }
  }
  // average eta and phi, take minimum r
  if (!pq.empty()) {
    double pmeta = 0, pmphi = 0, pr = pq.top().second[0], pE = 0;
    while (!pq.empty()) {
      auto top = pq.top();
      pq.pop();
      pr = std::min(pr, top.second[0]);
      double e = top.first.second;
      pmphi += top.second[1] * e;
      pmeta += top.second[2] * e;
      pE += e;
    }
    result.setPosition(
        edm4hep::utils::sphericalToVector(pr, edm4hep::utils::etaToAngle(pmeta / pE), pmphi / pE));
  }
  return result;
}

edm4eic::MutableCluster
ImagingClusterReco::reconstruct_cluster(const edm4eic::ProtoCluster& pcl) const {
  edm4eic::MutableCluster cluster;

  const auto& hits    = pcl.getHits();
  const auto& weights = pcl.getWeights();

  cluster.setType(Jug::Reco::ClusterType::kCluster3D);
  double energy      = 0.;
  double energyError = 0.;
  double time        = 0.;
  double timeError   = 0.;
  double meta        = 0.;
  double mx          = 0.;
  double my          = 0.;
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
    mx += hit.getPosition().x * energyWeight;
    my += hit.getPosition().y * energyWeight;
    r = std::min(edm4hep::utils::magnitude(hit.getPosition()), r);
    cluster.addToHits(hit);
  }
  cluster.setEnergy(energy);
  cluster.setEnergyError(std::sqrt(energyError));
  cluster.setTime(time / energy);
  cluster.setTimeError(std::sqrt(timeError) / energy);
  cluster.setNhits(hits.size());
  cluster.setPosition(
      edm4hep::utils::sphericalToVector(r, edm4hep::utils::etaToAngle(meta / energy),
                                        (mx != 0. || my != 0.) ? std::atan2(my, mx) : 0.));

  // Determine the final position using the configured estimator recipe(s).
  // positionSource and positionCompareSource are equally-configurable, both
  // computed via the same estimate_position() code path -- neither is a
  // hardcoded special case.
  //
  // If positionMaxDphi < 0, always use positionSource. Otherwise, compare
  // positionSource against positionCompareSource in azimuthal angle: if they
  // agree, use positionSource for the full position (x, y, z + covariance);
  // if they disagree, use positionCompareSource instead. The two are never
  // mixed component-by-component.
  if (cluster.getNhits() > 0) {
    auto srcEst = estimate_position(cluster.getHits(), m_cfg.positionSource);

    if (m_cfg.positionMaxDphi < 0) {
      cluster.setPosition(srcEst.getPosition());
      cluster.setPositionError(srcEst.getPositionError());
    } else {
      auto cmpEst = estimate_position(cluster.getHits(), m_cfg.positionCompareSource);

      const double dphi = edm4hep::utils::angleAzimuthal(srcEst.getPosition()) -
                          edm4hep::utils::angleAzimuthal(cmpEst.getPosition());
      const double dsphi = std::abs(sin(0.5 * dphi));

      const auto& chosen = (dsphi <= sin(0.5 * m_cfg.positionMaxDphi)) ? srcEst : cmpEst;
      cluster.setPosition(chosen.getPosition());
      cluster.setPositionError(chosen.getPositionError());
    }
  }

  // Shape parameters are calculated separately by CalorimeterClusterShape algorithm

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
    [[maybe_unused]] const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
    const truth::EventLinkNavigator<edm4eic::MCRecoCalorimeterHitLinkCollection>& link_nav,
    edm4eic::MCRecoClusterParticleLinkCollection* links,
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

  // bookkeeping maps for associated primaries
  std::map<edm4hep::MCParticle, double, CompareObjectID<edm4hep::MCParticle>> mapMCParToContrib;

  // --------------------------------------------------------------------------
  // 1. get associated sim hits and sum energy
  // --------------------------------------------------------------------------
  double eSimHitSum = 0.;
  for (auto clhit : cl.getHits()) {
    // Get linked sim hits using LinkNavigator
    const auto vecAssocSimHits = link_nav.linked(clhit.getRawHit());

    for (const auto& [simHit, weight] : vecAssocSimHits) {
      eSimHitSum += simHit.getEnergy();
    }

    debug("{} associated sim hits found for reco hit (cell ID = {})", vecAssocSimHits.size(),
          clhit.getCellID());

    // ------------------------------------------------------------------------
    // 2. loop through associated sim hits
    // ------------------------------------------------------------------------
    for (const auto& [simHit, weight] : vecAssocSimHits) {
      for (const auto& contrib : simHit.getContributions()) {
        // --------------------------------------------------------------------
        // grab primary responsible for contribution & increment relevant sum
        // --------------------------------------------------------------------
        edm4hep::MCParticle primary = truth::primaryFrom(contrib);
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

    truth::addWeightedRelation(
        cl, part, static_cast<float>(weight),
        gsl::not_null<edm4eic::MCRecoClusterParticleLinkCollection*>{links},
        gsl::not_null<edm4eic::MCRecoClusterParticleAssociationCollection*>{assocs});

    debug("Associated cluster #{} to MC Particle #{} (pid = {}, status = {}, energy = {}) with "
          "weight ({})",
          cl.getObjectID().index, part.getObjectID().index, part.getPDG(),
          part.getGeneratorStatus(), part.getEnergy(), weight);
  }
}

} // namespace eicrecon

