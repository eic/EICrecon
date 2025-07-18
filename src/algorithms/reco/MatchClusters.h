// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2024 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

// Takes a list of particles (presumed to be from tracking), and all available clusters.
// 1. Match clusters to their tracks using the mcID field
// 2. For unmatched clusters create neutrals and add to the particle list

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <stdint.h>
#include <map>
#include <string>
#include <string_view>

namespace eicrecon {

using MatchClustersAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection, edm4eic::ClusterCollection,
                      edm4eic::MCRecoClusterParticleAssociationCollection>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection,
                       edm4eic::MCRecoParticleAssociationCollection>>;

class MatchClusters : public MatchClustersAlgorithm {

public:
  MatchClusters(std::string_view name)
      : MatchClustersAlgorithm{name,
                               {"MCParticles", "CentralTracks", "CentralTrackAssociations",
                                "EcalClusters", "EcalClusterAssociations"},
                               {"ReconstructedParticles", "ReconstructedParticleAssociations"},
                               "Match tracks with clusters, and assign associations."} {}

  void init() final {};
  void process(const Input&, const Output&) const final;

private:
  // get a map of mcID --> cluster
  // input: clusters --> all clusters
  std::map<int, edm4eic::Cluster>
  indexedClusters(const edm4eic::ClusterCollection* clusters,
                  const edm4eic::MCRecoClusterParticleAssociationCollection* associations) const;

  // reconstruct a neutral cluster
  // (for now assuming the vertex is at (0,0,0))
  static edm4eic::MutableReconstructedParticle
  reconstruct_neutral(const edm4eic::Cluster* cluster, const double mass, const int32_t pdg);
};

} // namespace eicrecon
