// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackCollection.h>
#include <cstddef>
#include <memory>

#include "algorithms/particle/ChargedCandidateMaker.h"

TEST_CASE("the ChargedCandidateMaker algorithm runs", "[ChargedCandidateMaker]") {
  eicrecon::ChargedCandidateMaker algo("test");

  // initialize algorithm
  algo.level(algorithms::LogLevel::kDebug);
  algo.init();

  SECTION("empty input produces empty output") {
    auto matches   = std::make_unique<edm4eic::TrackClusterMatchCollection>();
    auto particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    algo.process({matches.get()}, {particles.get()});

    REQUIRE(particles->size() == 0);
  }

  SECTION("single match produces one particle with one track and one cluster") {
    // Create backing collections that own the objects
    auto tracks   = std::make_unique<edm4eic::TrackCollection>();
    auto clusters = std::make_unique<edm4eic::ClusterCollection>();
    auto matches  = std::make_unique<edm4eic::TrackClusterMatchCollection>();

    auto track   = tracks->create();
    auto cluster = clusters->create();

    auto match = matches->create();
    match.setTrack(track);
    match.setCluster(cluster);
    match.setWeight(1.0f);

    auto particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    algo.process({matches.get()}, {particles.get()});

    REQUIRE(particles->size() == 1);

    auto particle = particles->at(0);
    REQUIRE(particle.tracks_size() == 1);
    REQUIRE(particle.clusters_size() == 1);
    REQUIRE(particle.getTracks(0) == track);
    REQUIRE(particle.getClusters(0) == cluster);
  }

  SECTION("two matches with the same track produce one particle with two clusters") {
    auto tracks   = std::make_unique<edm4eic::TrackCollection>();
    auto clusters = std::make_unique<edm4eic::ClusterCollection>();
    auto matches  = std::make_unique<edm4eic::TrackClusterMatchCollection>();

    auto track    = tracks->create();
    auto clusterA = clusters->create();
    auto clusterB = clusters->create();

    auto matchA = matches->create();
    matchA.setTrack(track);
    matchA.setCluster(clusterA);
    matchA.setWeight(1.0f);

    auto matchB = matches->create();
    matchB.setTrack(track);
    matchB.setCluster(clusterB);
    matchB.setWeight(0.5f);

    auto particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    algo.process({matches.get()}, {particles.get()});

    REQUIRE(particles->size() == 1);

    auto particle = particles->at(0);
    REQUIRE(particle.tracks_size() == 1);
    REQUIRE(particle.clusters_size() == 2);
    REQUIRE(particle.getTracks(0) == track);
  }

  SECTION("two matches with different tracks produce two particles") {
    auto tracks   = std::make_unique<edm4eic::TrackCollection>();
    auto clusters = std::make_unique<edm4eic::ClusterCollection>();
    auto matches  = std::make_unique<edm4eic::TrackClusterMatchCollection>();

    auto trackA   = tracks->create();
    auto trackB   = tracks->create();
    auto clusterA = clusters->create();
    auto clusterB = clusters->create();

    auto matchA = matches->create();
    matchA.setTrack(trackA);
    matchA.setCluster(clusterA);
    matchA.setWeight(1.0f);

    auto matchB = matches->create();
    matchB.setTrack(trackB);
    matchB.setCluster(clusterB);
    matchB.setWeight(1.0f);

    auto particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    algo.process({matches.get()}, {particles.get()});

    REQUIRE(particles->size() == 2);

    // Each particle should have exactly one track and one cluster
    for (std::size_t i = 0; i < particles->size(); ++i) {
      auto particle = particles->at(i);
      REQUIRE(particle.tracks_size() == 1);
      REQUIRE(particle.clusters_size() == 1);
    }
  }
}
