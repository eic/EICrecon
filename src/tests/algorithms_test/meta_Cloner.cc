// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <podio/RelationRange.h>
#include <memory>
#include <string>

#include "algorithms/meta/Cloner.h"
#include "algorithms/meta/ClonerConfig.h"

TEST_CASE("the Cloner algorithm runs", "[Cloner]") {
  // Full collection with a beam particle that has a daughter
  auto particles = std::make_unique<edm4hep::MCParticleCollection>();
  auto beam      = particles->create();
  beam.setPDG(11);
  beam.setGeneratorStatus(4);
  beam.setMomentum({0.0, 0.0, -10.0});
  auto daughter = particles->create();
  daughter.setPDG(11);
  daughter.setGeneratorStatus(1);
  beam.addToDaughters(daughter);
  daughter.addToParents(beam);

  // Subset collection with only the beam particle
  auto subset = std::make_unique<edm4hep::MCParticleCollection>();
  subset->setSubsetCollection();
  subset->push_back(beam);

  using AlgoT = eicrecon::Cloner<edm4hep::MCParticle>;

  SECTION("clones with relations by default") {
    AlgoT algo("cloner_default");
    algo.level(algorithms::LogLevel::kDebug);
    algo.init();

    auto output = std::make_unique<edm4hep::MCParticleCollection>();
    algo.process({subset.get()}, {output.get()});

    REQUIRE(!output->isSubsetCollection());
    REQUIRE(output->size() == 1);
    REQUIRE(output->at(0).getPDG() == 11);
    REQUIRE(output->at(0).getGeneratorStatus() == 4);
    REQUIRE(output->at(0).getMomentum().z == -10.0);
    REQUIRE(output->at(0).getDaughters().size() == 1);
  }

  SECTION("clones without relations when cloneRelations is false") {
    AlgoT algo("cloner_no_relations");
    algo.level(algorithms::LogLevel::kDebug);
    algo.applyConfig(eicrecon::ClonerConfig{.cloneRelations = false});
    algo.init();

    auto output = std::make_unique<edm4hep::MCParticleCollection>();
    algo.process({subset.get()}, {output.get()});

    REQUIRE(!output->isSubsetCollection());
    REQUIRE(output->size() == 1);
    REQUIRE(output->at(0).getPDG() == 11);
    REQUIRE(output->at(0).getGeneratorStatus() == 4);
    REQUIRE(output->at(0).getMomentum().z == -10.0);
    REQUIRE(output->at(0).getDaughters().empty());
    REQUIRE(output->at(0).getParents().empty());
  }
}
