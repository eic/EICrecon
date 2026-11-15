// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>

#include "algorithms/calorimetry/EnergyPositionClusterMerger.h"

TEST_CASE("EnergyPositionClusterMerger handles optional associations",
          "[EnergyPositionClusterMerger]") {
  eicrecon::EnergyPositionClusterMerger algo("EnergyPositionClusterMerger");
  algo.level(algorithms::LogLevel::kDebug);
  algo.init();

  // Truth particles
  edm4hep::MCParticleCollection mcparticles;
  auto mc_energy   = mcparticles.create();
  auto mc_position = mcparticles.create();

  // One energy cluster and one matching position cluster
  edm4eic::ClusterCollection energy_clusters;
  auto ec = energy_clusters.create();
  ec.setEnergy(10.0);
  ec.setPosition({100.0, 0.0, 1000.0});

  edm4eic::ClusterCollection position_clusters;
  auto pc = position_clusters.create();
  pc.setEnergy(9.5);
  pc.setPosition({100.0, 0.0, 1000.0});

  edm4eic::MCRecoClusterParticleAssociationCollection energy_assocs;
  auto ea = energy_assocs.create();
  ea.setRec(ec);
  ea.setSim(mc_energy);
  ea.setWeight(1.0);

  edm4eic::MCRecoClusterParticleAssociationCollection position_assocs;
  auto pa = position_assocs.create();
  pa.setRec(pc);
  pa.setSim(mc_position);
  pa.setWeight(1.0);

  auto merged_clusters = std::make_unique<edm4eic::ClusterCollection>();
  auto merged_links    = std::make_unique<edm4eic::MCRecoClusterParticleLinkCollection>();
  auto merged_assocs   = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

  SECTION("both association collections available") {
    algo.process({&energy_clusters, &energy_assocs, &position_clusters, &position_assocs},
                 {merged_clusters.get(), merged_links.get(), merged_assocs.get()});
    REQUIRE(merged_clusters->size() == 1);
    REQUIRE(merged_assocs->size() == 2);
    REQUIRE(merged_links->size() == 2);
    REQUIRE((*merged_assocs)[0].getWeight() == 0.5);
    REQUIRE((*merged_assocs)[1].getWeight() == 0.5);
  }

  SECTION("only energy association collection available") {
    algo.process({&energy_clusters, &energy_assocs, &position_clusters, nullptr},
                 {merged_clusters.get(), merged_links.get(), merged_assocs.get()});
    REQUIRE(merged_clusters->size() == 1);
    REQUIRE(merged_assocs->size() == 1);
    REQUIRE(merged_links->size() == 1);
    REQUIRE((*merged_assocs)[0].getSim().getObjectID() == mc_energy.getObjectID());
    REQUIRE((*merged_assocs)[0].getWeight() == 1.0);
  }

  SECTION("only position association collection available") {
    algo.process({&energy_clusters, nullptr, &position_clusters, &position_assocs},
                 {merged_clusters.get(), merged_links.get(), merged_assocs.get()});
    REQUIRE(merged_clusters->size() == 1);
    REQUIRE(merged_assocs->size() == 1);
    REQUIRE(merged_links->size() == 1);
    REQUIRE((*merged_assocs)[0].getSim().getObjectID() == mc_position.getObjectID());
  }

  SECTION("no association collections available") {
    algo.process({&energy_clusters, nullptr, &position_clusters, nullptr},
                 {merged_clusters.get(), merged_links.get(), merged_assocs.get()});
    REQUIRE(merged_clusters->size() == 1);
    REQUIRE(merged_assocs->empty());
    REQUIRE(merged_links->empty());
  }
}
