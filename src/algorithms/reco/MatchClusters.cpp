// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

// Takes a list of particles (presumed to be from tracking), and all available clusters.
// 1. Match clusters to their tracks using the mcID field
// 2. For unmatched clusters create neutrals and add to the particle list

#include <algorithm>
#include <cmath>

#include <fmt/format.h>

#include "Gaudi/Algorithm.h"
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/Producer.h"
#include "GaudiAlg/Transformer.h"
#include "GaudiKernel/RndmGenerators.h"

#include "JugBase/DataHandle.h"

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/ClusterCollection.h"
#include "edm4eic/MCRecoClusterParticleAssociationCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/TrackParametersCollection.h"
#include "edm4eic/vector_utils.h"

namespace Jug::Fast {

class MatchClusters : public GaudiAlgorithm {
private:
  // input data
  DataHandle<edm4hep::MCParticleCollection> m_inputMCParticles{"MCParticles", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4eic::ReconstructedParticleCollection> m_inputParticles{"ReconstructedChargedParticles",
                                                                    Gaudi::DataHandle::Reader, this};
  DataHandle<edm4eic::MCRecoParticleAssociationCollection> m_inputParticlesAssoc{"ReconstructedChargedParticlesAssoc",
                                                                    Gaudi::DataHandle::Reader, this};
  Gaudi::Property<std::vector<std::string>> m_inputClusters{this, "inputClusters", {}, "Clusters to be aggregated"};
  Gaudi::Property<std::vector<std::string>> m_inputClustersAssoc{this, "inputClustersAssoc", {}, "Cluster associations to be aggregated"};
  std::vector<DataHandle<edm4eic::ClusterCollection>*> m_inputClustersCollections;
  std::vector<DataHandle<edm4eic::MCRecoClusterParticleAssociationCollection>*> m_inputClustersAssocCollections;

  // output data
  DataHandle<edm4eic::ReconstructedParticleCollection> m_outputParticles{"ReconstructedParticles",
                                                                     Gaudi::DataHandle::Writer, this};
  DataHandle<edm4eic::MCRecoParticleAssociationCollection> m_outputParticlesAssoc{"ReconstructedParticlesAssoc",
                                                                     Gaudi::DataHandle::Writer, this};

public:
  MatchClusters(const std::string& name, ISvcLocator* svcLoc)
      : GaudiAlgorithm(name, svcLoc) {
    declareProperty("inputMCParticles", m_inputMCParticles, "MCParticles");
    declareProperty("inputParticles", m_inputParticles, "ReconstructedChargedParticles");
    declareProperty("inputParticlesAssoc", m_inputParticlesAssoc, "ReconstructedChargedParticlesAssoc");
    declareProperty("outputParticles", m_outputParticles, "ReconstructedParticles");
    declareProperty("outputParticlesAssoc", m_outputParticlesAssoc, "ReconstructedParticlesAssoc");
  }

  StatusCode initialize() override {
    if (GaudiAlgorithm::initialize().isFailure()) {
      return StatusCode::FAILURE;
    }
    m_inputClustersCollections = getClusterCollections(m_inputClusters);
    m_inputClustersAssocCollections = getClusterAssociations(m_inputClustersAssoc);
    return StatusCode::SUCCESS;
  }
  StatusCode execute() override {
    if (msgLevel(MSG::DEBUG)) {
      debug() << "Processing cluster info for new event" << endmsg;
    }
    // input collection
    const auto& mcparticles  = *(m_inputMCParticles.get());
    const auto& inparts      = *(m_inputParticles.get());
    const auto& inpartsassoc = *(m_inputParticlesAssoc.get());
    auto& outparts           = *(m_outputParticles.createAndPut());
    auto& outpartsassoc      = *(m_outputParticlesAssoc.createAndPut());

    if (msgLevel(MSG::DEBUG)) {
      debug() << "Step 0/2: Getting indexed list of clusters..." << endmsg;
    }

    // get an indexed map of all clusters
    auto clusterMap = indexedClusters(m_inputClustersCollections, m_inputClustersAssocCollections);

    // 1. Loop over all tracks and link matched clusters where applicable
    // (removing matched clusters from the cluster maps)
    if (msgLevel(MSG::DEBUG)) {
      debug() << "Step 1/2: Matching clusters to charged particles..." << endmsg;
    }
    for (const auto& inpart: inparts) {
      if (msgLevel(MSG::DEBUG)) {
        debug() << " --> Processing charged particle " << inpart.getObjectID().index
                << ", PDG: " << inpart.getPDG()
                << ", energy: " << inpart.getEnergy()
                << endmsg;
      }

      auto outpart = inpart.clone();
      outparts.push_back(outpart);

      int mcID = -1;

      // find associated particle
      for (const auto& assoc: inpartsassoc) {
        if (assoc.getRec() == inpart) {
          mcID = assoc.getSimID();
          break;
        }
      }

      if (msgLevel(MSG::VERBOSE)) {
        verbose() << "    --> Found particle with mcID " << mcID << endmsg;
      }

      if (mcID < 0) {
        if (msgLevel(MSG::DEBUG)) {
          debug() << "    --> cannot match track without associated mcID" << endmsg;
        }
        continue;
      }

      if (clusterMap.count(mcID)) {
        const auto& clus = clusterMap[mcID];
        if (msgLevel(MSG::DEBUG)) {
          debug() << "    --> found matching cluster with energy: " << clus.getEnergy() << endmsg;
        }
        clusterMap.erase(mcID);
      }

      // create truth associations
      auto assoc = outpartsassoc.create();
      assoc.setRecID(outpart.getObjectID().index);
      assoc.setSimID(mcID);
      assoc.setWeight(1.0);
      assoc.setRec(outpart);
      //assoc.setSim(mcparticles[mcID]);
    }

    // 2. Now loop over all remaining clusters and add neutrals. Also add in Hcal energy
    // if a matching cluster is available
    if (msgLevel(MSG::DEBUG)) {
      debug() << "Step 2/2: Creating neutrals for remaining clusters..." << endmsg;
    }
    for (const auto& [mcID, clus] : clusterMap) {
      if (msgLevel(MSG::DEBUG)) {
        debug() << " --> Processing unmatched cluster with energy: " << clus.getEnergy()
                << endmsg;
      }

      // get mass/PDG from mcparticles, 0 (unidentified) in case the matched particle is charged.
      const auto& mc    = mcparticles[mcID];
      const double mass = (!mc.getCharge()) ? mc.getMass() : 0;
      const int32_t pdg = (!mc.getCharge()) ? mc.getPDG() : 0;
      if (msgLevel(MSG::DEBUG)) {
        if (mc.getCharge()) {
          debug() << "   --> associated mcparticle is not a neutral (PDG: " << mc.getPDG()
                  << "), setting the reconstructed particle ID to 0 (unidentified)" << endmsg;
        }
        debug() << "   --> found matching associated mcparticle with PDG: " << pdg << ", energy: " << mc.getEnergy()
                << endmsg;
      }

      // Reconstruct our neutrals and add them to the list
      const auto outpart = reconstruct_neutral(clus, mass, pdg);
      if (msgLevel(MSG::DEBUG)) {
        debug() << " --> Reconstructed neutral particle with PDG: " << outpart.getPDG()
                << ", energy: " << outpart.getEnergy()
                << endmsg;
      }
      outparts.push_back(outpart);

      // Create truth associations
      auto assoc = outpartsassoc.create();
      assoc.setRecID(outpart.getObjectID().index);
      assoc.setSimID(mcID);
      assoc.setWeight(1.0);
      assoc.setRec(outpart);
      //assoc.setSim(mcparticles[mcID]);
    }
    return StatusCode::SUCCESS;
  }

private:
  std::vector<DataHandle<edm4eic::ClusterCollection>*> getClusterCollections(const std::vector<std::string>& cols) {
    std::vector<DataHandle<edm4eic::ClusterCollection>*> ret;
    for (const auto& colname : cols) {
      debug() << "initializing cluster collection: " << colname << endmsg;
      ret.push_back(new DataHandle<edm4eic::ClusterCollection>{colname, Gaudi::DataHandle::Reader, this});
    }
    return ret;
  }

  std::vector<DataHandle<edm4eic::MCRecoClusterParticleAssociationCollection>*> getClusterAssociations(const std::vector<std::string>& cols) {
    std::vector<DataHandle<edm4eic::MCRecoClusterParticleAssociationCollection>*> ret;
    for (const auto& colname : cols) {
      debug() << "initializing cluster association collection: " << colname << endmsg;
      ret.push_back(new DataHandle<edm4eic::MCRecoClusterParticleAssociationCollection>{colname, Gaudi::DataHandle::Reader, this});
    }
    return ret;
  }

  // get a map of mcID --> cluster
  // input: cluster_collections --> list of handles to all cluster collections
  std::map<int, edm4eic::Cluster>
  indexedClusters(
      const std::vector<DataHandle<edm4eic::ClusterCollection>*>& cluster_collections,
      const std::vector<DataHandle<edm4eic::MCRecoClusterParticleAssociationCollection>*>& associations_collections
  ) const {
    std::map<int, edm4eic::Cluster> matched = {};

    // loop over cluster collections
    for (const auto& cluster_handle : cluster_collections) {
      const auto& clusters = *(cluster_handle->get());

      // loop over clusters
      for (const auto& cluster : clusters) {

        int mcID = -1;

        // loop over association collections
        for (const auto& associations_handle : associations_collections) {
          const auto& associations = *(associations_handle->get());

          // find associated particle
          for (const auto& assoc : associations) {
            if (assoc.getRec() == cluster) {
              mcID = assoc.getSimID();
              break;
            }
          }

          // found associated particle
          if (mcID != -1) {
            break;
          }
        }

        if (msgLevel(MSG::VERBOSE)) {
          verbose() << " --> Found cluster with mcID " << mcID << " and energy "
                    << cluster.getEnergy() << endmsg;
        }

        if (mcID < 0) {
          if (msgLevel(MSG::VERBOSE)) {
            verbose() << "   --> WARNING: no valid MC truth link found, skipping cluster..." << endmsg;
          }
          continue;
        }

        const bool duplicate = matched.count(mcID);
        if (duplicate) {
          if (msgLevel(MSG::VERBOSE)) {
            verbose() << "   --> WARNING: this is a duplicate mcID, keeping the higher energy cluster" << endmsg;
          }
          if (cluster.getEnergy() < matched[mcID].getEnergy()) {
            continue;
          }
        }
        matched[mcID] = cluster;
      }
    }
    return matched;
  }

  // reconstruct a neutral cluster
  // (for now assuming the vertex is at (0,0,0))
  edm4eic::ReconstructedParticle reconstruct_neutral(const edm4eic::Cluster& clus, const double mass,
                                                 const int32_t pdg) const {
    const float energy = clus.getEnergy();
    const float p = energy < mass ? 0 : std::sqrt(energy * energy - mass * mass);
    const auto position = clus.getPosition();
    const auto momentum = p * (position / edm4eic::magnitude(position));
    // setup our particle
    edm4eic::MutableReconstructedParticle part;
    part.setMomentum(momentum);
    part.setPDG(pdg);
    part.setCharge(0);
    part.setEnergy(energy);
    part.setMass(mass);
    return part;
  }
}; // namespace Jug::Fast

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
DECLARE_COMPONENT(MatchClusters)

} // namespace Jug::Fast
