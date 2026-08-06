// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, ePIC Collaboration

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <gsl/pointers>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

#include "algorithms/particle_flow/CaloRemnantCombiner.h"
#include "algorithms/particle_flow/CaloRemnantCombinerConfig.h"

namespace {

//! Create a cluster at a given (eta, phi) with a given energy at radius r
edm4eic::MutableCluster make_cluster(edm4eic::ClusterCollection& coll, float energy, float eta,
                                     float phi, float r = 2000.F) {
  auto cluster = coll.create();
  cluster.setEnergy(energy);
  cluster.setPosition({r * std::cos(phi) / std::cosh(eta), r * std::sin(phi) / std::cosh(eta),
                       r * std::tanh(eta)});
  return cluster;
}

} // namespace

TEST_CASE("the CaloRemnantCombiner algorithm runs", "[CaloRemnantCombiner]") {
  eicrecon::CaloRemnantCombiner algo("test");

  eicrecon::CaloRemnantCombinerConfig cfg;
  cfg.ecalDeltaR = 0.03;
  cfg.hcalDeltaR = 0.15;

  algo.applyConfig(cfg);
  algo.level(algorithms::LogLevel::kDebug);
  algo.init();

  auto ecal       = std::make_unique<edm4eic::ClusterCollection>();
  auto hcal       = std::make_unique<edm4eic::ClusterCollection>();
  auto candidates = std::make_unique<edm4eic::ReconstructedParticleCollection>();

  std::vector<gsl::not_null<const edm4eic::ClusterCollection*>> input = {ecal.get(), hcal.get()};

  SECTION("empty input produces empty output") {
    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 0);
  }

  SECTION("single ecal cluster produces one candidate") {
    auto cluster = make_cluster(*ecal, 1.0F, 0.5F, 0.5F);

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 1);
    REQUIRE(candidates->at(0).clusters_size() == 1);
    REQUIRE(candidates->at(0).getClusters(0) == cluster);
  }

  SECTION("single hcal cluster produces one candidate") {
    auto cluster = make_cluster(*hcal, 1.0F, 0.5F, 0.5F);

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 1);
    REQUIRE(candidates->at(0).clusters_size() == 1);
    REQUIRE(candidates->at(0).getClusters(0) == cluster);
  }

  SECTION("hcal cluster within hcalDeltaR of ecal seed is merged") {
    auto ecal_cluster = make_cluster(*ecal, 1.0F, 0.5F, 0.5F);
    auto hcal_cluster = make_cluster(*hcal, 2.0F, 0.5F + 0.1F, 0.5F); // dR = 0.1 < 0.15

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 1);
    REQUIRE(candidates->at(0).clusters_size() == 2);
    REQUIRE(candidates->at(0).getClusters(0) == ecal_cluster);
    REQUIRE(candidates->at(0).getClusters(1) == hcal_cluster);
  }

  SECTION("hcal cluster outside hcalDeltaR of ecal seed becomes its own candidate") {
    auto ecal_cluster = make_cluster(*ecal, 1.0F, 0.5F, 0.5F);
    auto hcal_cluster = make_cluster(*hcal, 2.0F, 0.5F + 0.5F, 0.5F); // dR = 0.5 > 0.15

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 2);
    // ecal-seeded candidate comes first
    REQUIRE(candidates->at(0).clusters_size() == 1);
    REQUIRE(candidates->at(0).getClusters(0) == ecal_cluster);
    // hcal-seeded candidate follows
    REQUIRE(candidates->at(1).clusters_size() == 1);
    REQUIRE(candidates->at(1).getClusters(0) == hcal_cluster);
  }

  SECTION("nearby ecal clusters are merged into one candidate seeded by highest energy") {
    auto low  = make_cluster(*ecal, 1.0F, 0.5F + 0.02F, 0.5F); // dR = 0.02 < 0.03
    auto high = make_cluster(*ecal, 5.0F, 0.5F, 0.5F);

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 1);
    REQUIRE(candidates->at(0).clusters_size() == 2);
    // clusters are attached in descending energy order (seed first)
    REQUIRE(candidates->at(0).getClusters(0) == high);
    REQUIRE(candidates->at(0).getClusters(1) == low);
  }

  SECTION("distant ecal clusters produce separate candidates") {
    auto low  = make_cluster(*ecal, 1.0F, -0.5F, 0.5F);
    auto high = make_cluster(*ecal, 5.0F, 0.5F, 0.5F);

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 2);
    // highest energy cluster is seeded first
    REQUIRE(candidates->at(0).clusters_size() == 1);
    REQUIRE(candidates->at(0).getClusters(0) == high);
    REQUIRE(candidates->at(1).clusters_size() == 1);
    REQUIRE(candidates->at(1).getClusters(0) == low);
  }

  SECTION("nearby hcal clusters are merged into one candidate in phase 2") {
    auto high = make_cluster(*hcal, 5.0F, 0.5F, 0.5F);
    auto low  = make_cluster(*hcal, 1.0F, 0.5F, 0.5F + 0.1F); // dR = 0.1 < 0.15

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 1);
    REQUIRE(candidates->at(0).clusters_size() == 2);
    REQUIRE(candidates->at(0).getClusters(0) == high);
    REQUIRE(candidates->at(0).getClusters(1) == low);
  }

  SECTION("clusters are merged across the phi = +/- pi boundary") {
    constexpr float pi = M_PI;

    auto seed  = make_cluster(*ecal, 5.0F, 0.5F, pi - 0.01F);
    auto other = make_cluster(*ecal, 1.0F, 0.5F, -pi + 0.01F); // wrapped dphi = 0.02 < 0.03

    algo.process({input}, {candidates.get()});

    REQUIRE(candidates->size() == 1);
    REQUIRE(candidates->at(0).clusters_size() == 2);
    REQUIRE(candidates->at(0).getClusters(0) == seed);
    REQUIRE(candidates->at(0).getClusters(1) == other);
  }

  SECTION("every input cluster ends up in exactly one candidate") {
    make_cluster(*ecal, 1.0F, -1.0F, 0.0F);
    make_cluster(*ecal, 2.0F, 0.0F, 1.0F);
    make_cluster(*ecal, 3.0F, 1.0F, 2.0F);
    make_cluster(*hcal, 1.5F, -1.0F, 0.0F);
    make_cluster(*hcal, 2.5F, 2.0F, -2.0F);

    algo.process({input}, {candidates.get()});

    std::size_t n_clusters = 0;
    for (const auto& candidate : *candidates) {
      n_clusters += candidate.clusters_size();
    }
    REQUIRE(n_clusters == ecal->size() + hcal->size());
  }
}
