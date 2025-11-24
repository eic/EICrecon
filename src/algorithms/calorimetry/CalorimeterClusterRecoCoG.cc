// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Dhevan Gangadharan, Derek Anderson

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicking energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */

#include <Evaluator/DD4hepUnits.h>
#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cctype>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <map>
#include <optional>
#include <vector>

#include "CalorimeterClusterRecoCoG.h"
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"
#include "MCTools.h"

namespace eicrecon {

using namespace dd4hep;

void CalorimeterClusterRecoCoG::init() {
  // select weighting method
  std::string ew = m_cfg.energyWeight;
  // make it case-insensitive
  std::ranges::transform(ew, ew.begin(), [](char s) { return std::tolower(s); });
  auto it = weightMethods.find(ew);
  if (it == weightMethods.end()) {
    error("Cannot find energy weighting method {}, choose one from [{}]", m_cfg.energyWeight,
          boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "));
    return;
  }
  weightFunc = it->second;
}

void CalorimeterClusterRecoCoG::process(const CalorimeterClusterRecoCoG::Input& input,
                                        const CalorimeterClusterRecoCoG::Output& output) const {
  const auto [proto, mchitassociations] = input;
  auto [clusters, associations]         = output;

  for (const auto& pcl : *proto) {
    // skip protoclusters with no hits
    if (pcl.hits_size() == 0) {
      continue;
    }

    auto cl_opt = reconstruct(pcl);
    if (!cl_opt.has_value()) {
      continue;
    }
    auto cl = *std::move(cl_opt);

    debug("{} hits: {} GeV, ({}, {}, {})", cl.getNhits(), cl.getEnergy() / dd4hep::GeV,
          cl.getPosition().x / dd4hep::mm, cl.getPosition().y / dd4hep::mm,
          cl.getPosition().z / dd4hep::mm);
    clusters->push_back(cl);

    // If sim hits are available, associate cluster with MCParticle
    if (mchitassociations->empty()) {
      debug("Provided MCRecoCalorimeterHitAssociation collection is empty. No truth associations "
            "will be performed.");
      continue;
    }
    associate(cl, mchitassociations, associations);
  }
}

std::optional<edm4eic::MutableCluster>
CalorimeterClusterRecoCoG::reconstruct(const edm4eic::ProtoCluster& pcl) const {
  edm4eic::MutableCluster cl;
  cl.setNhits(pcl.hits_size());

  debug("hit size = {}", pcl.hits_size());

  // no hits
  if (pcl.hits_size() == 0) {
    return {};
  }

  // calculate total energy, find the cell with the maximum energy deposit
  float totalE = 0.;
  // Used to optionally constrain the cluster eta to those of the contributing hits
  float minHitEta = std::numeric_limits<float>::max();
  float maxHitEta = std::numeric_limits<float>::min();
  auto time       = 0;
  auto timeError  = 0;
  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    debug("hit energy = {} hit weight: {}", hit.getEnergy(), weight);
    auto energy = hit.getEnergy() * weight;
    totalE += energy;
    time += (hit.getTime() - time) * energy / totalE;
    cl.addToHits(hit);
    cl.addToHitContributions(energy);
    const float eta = edm4hep::utils::eta(hit.getPosition());
    minHitEta       = std::min(eta, minHitEta);
    maxHitEta       = std::max(eta, maxHitEta);
  }
  cl.setEnergy(totalE / m_cfg.sampFrac);
  cl.setEnergyError(0.);
  cl.setTime(time);
  cl.setTimeError(timeError);

  // center of gravity with logarithmic weighting
  float tw = 0.;
  auto v   = cl.getPosition();

  double logWeightBase = m_cfg.logWeightBase;
  if (!m_cfg.logWeightBaseCoeffs.empty()) {
    double l      = std::log(cl.getEnergy() / m_cfg.logWeightBase_Eref);
    logWeightBase = 0;
    for (std::size_t i = 0; i < m_cfg.logWeightBaseCoeffs.size(); i++) {
      logWeightBase += m_cfg.logWeightBaseCoeffs[i] * pow(l, i);
    }
  }

  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    //      _DBG_<<" -- weight = " << weight << "  E=" << hit.getEnergy() << " totalE=" <<totalE << " log(E/totalE)=" << std::log(hit.getEnergy()/totalE) << std::endl;
    float w = weightFunc(hit.getEnergy() * weight, totalE, logWeightBase, 0);
    tw += w;
    v = v + (hit.getPosition() * w);
  }
  if (tw == 0.) {
    warning("zero total weights encountered, you may want to adjust your weighting parameter.");
    return {};
  }
  cl.setPosition(v / tw);
  cl.setPositionError({}); // @TODO: Covariance matrix

  // Optionally constrain the cluster to the hit eta values
  if (m_cfg.enableEtaBounds) {
    const bool overflow  = (edm4hep::utils::eta(cl.getPosition()) > maxHitEta);
    const bool underflow = (edm4hep::utils::eta(cl.getPosition()) < minHitEta);
    if (overflow || underflow) {
      const double newEta   = overflow ? maxHitEta : minHitEta;
      const double newTheta = edm4hep::utils::etaToAngle(newEta);
      const double newR     = edm4hep::utils::magnitude(cl.getPosition());
      const double newPhi   = edm4hep::utils::angleAzimuthal(cl.getPosition());
      cl.setPosition(edm4hep::utils::sphericalToVector(newR, newTheta, newPhi));
      debug("Bound cluster position to contributing hits due to {}",
            (overflow ? "overflow" : "underflow"));
    }
  }
  return cl;
}

void CalorimeterClusterRecoCoG::associate(
    const edm4eic::Cluster& cl,
    const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
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
    }
    return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
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

    for (const auto& hitAssoc : *mchitassociations) {
      // if found corresponding raw hit, add sim hit to vector
      // and increment energy sum
      if (clhit.getRawHit() == hitAssoc.getRawHit()) {
        vecAssocSimHits.push_back(hitAssoc.getSimHit());
        eSimHitSum += vecAssocSimHits.back().getEnergy();
      }
    }
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
        edm4hep::MCParticle primary = MCTools::lookup_primary(contrib);
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

} // namespace eicrecon
