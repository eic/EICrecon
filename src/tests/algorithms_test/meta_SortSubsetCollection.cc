// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/ClusterCollection.h>
#include <memory>

#include "algorithms/meta/SortSubsetCollection.h"

namespace {

float cluster_energy(const edm4eic::Cluster& cluster) { return cluster.getEnergy(); }

} // namespace

TEST_CASE("the SortSubsetCollection algorithm runs", "[SortSubsetCollection]") {
  auto make_input = []() {
    auto input = std::make_unique<edm4eic::ClusterCollection>();

    auto c0 = input->create();
    c0.setEnergy(3.0f);
    c0.setPosition({4.0f, 0.0f, 0.0f});

    auto c1 = input->create();
    c1.setEnergy(1.0f);
    c1.setPosition({1.0f, 0.0f, 0.0f});

    auto c2 = input->create();
    c2.setEnergy(2.0f);
    c2.setPosition({3.0f, 0.0f, 0.0f});

    auto c3 = input->create();
    c3.setEnergy(2.0f);
    c3.setPosition({2.0f, 0.0f, 0.0f});

    return input;
  };

  SECTION("sorts by accessor function") {
    using AlgoT = eicrecon::SortSubsetCollection<edm4eic::Cluster, decltype(&cluster_energy)>;
    AlgoT algo("sort_by_function", &cluster_energy);
    algo.level(algorithms::LogLevel::kDebug);
    algo.init();

    auto input  = make_input();
    auto output = std::make_unique<edm4eic::ClusterCollection>();

    algo.process({input.get()}, {output.get()});

    REQUIRE(output->size() == 4);
    REQUIRE(output->at(0).getEnergy() == 1.0f);
    REQUIRE(output->at(1).getEnergy() == 2.0f);
    REQUIRE(output->at(2).getEnergy() == 2.0f);
    REQUIRE(output->at(3).getEnergy() == 3.0f);
    REQUIRE(output->at(1).getPosition().x == 3.0f);
    REQUIRE(output->at(2).getPosition().x == 2.0f);
  }

  SECTION("sorts by lambda") {
    auto sort_key = [](const edm4eic::Cluster& cluster) { return cluster.getPosition().x; };
    using AlgoT   = eicrecon::SortSubsetCollection<edm4eic::Cluster, decltype(sort_key)>;

    AlgoT algo("sort_by_lambda", sort_key);
    algo.level(algorithms::LogLevel::kDebug);
    algo.init();

    auto input  = make_input();
    auto output = std::make_unique<edm4eic::ClusterCollection>();

    algo.process({input.get()}, {output.get()});

    REQUIRE(output->size() == 4);
    REQUIRE(output->at(0).getPosition().x == 1.0f);
    REQUIRE(output->at(1).getPosition().x == 2.0f);
    REQUIRE(output->at(2).getPosition().x == 3.0f);
    REQUIRE(output->at(3).getPosition().x == 4.0f);
  }
}
