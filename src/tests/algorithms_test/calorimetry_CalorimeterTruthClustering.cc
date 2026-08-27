// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <cmath>
#include <cstddef>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <set>
#include <vector>

#include "algorithms/calorimetry/CalorimeterTruthClustering.h"

TEST_CASE("the CalorimeterTruthClustering algorithm runs", "[CalorimeterTruthClustering]") {

  eicrecon::CalorimeterTruthClustering algo_clustering("calorimeterTruthClustering");
  algo_clustering.level(algorithms::LogLevel::kDebug);
  algo_clustering.init();

  SECTION("empty input produces empty output") {
    auto empty_rec_calo_hit_coll = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto empty_mc_rec_hit_link_coll =
        std::make_unique<edm4eic::MCRecoCalorimeterHitLinkCollection>();
    auto empty_truth_clust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
    algo_clustering.process({empty_rec_calo_hit_coll.get(), empty_mc_rec_hit_link_coll.get()},
                            {empty_truth_clust_coll.get()});
    REQUIRE(empty_truth_clust_coll->size() == 0);
  }

  edm4hep::MutableMCParticle par_a = edm4hep::MutableMCParticle();
  par_a.setPDG(11);
  par_a.setCharge(-1.0);
  par_a.setMass(0.000511);
  par_a.setGeneratorStatus(1);

  edm4hep::MutableMCParticle par_b = edm4hep::MutableMCParticle();
  par_b.setPDG(22);
  par_b.setCharge(0.0);
  par_b.setMass(0.0);
  par_b.setGeneratorStatus(1);

  edm4hep::MutableMCParticle par_c = edm4hep::MutableMCParticle();
  par_c.setPDG(111);
  par_c.setCharge(0.0);
  par_c.setMass(0.134);
  par_c.setGeneratorStatus(1);

  edm4hep::MutableMCParticle par_c_c = edm4hep::MutableMCParticle();
  par_c_c.setPDG(22);
  par_c_c.setCharge(0.0);
  par_c_c.setMass(0.0);
  par_c_c.setGeneratorStatus(0);
  par_c_c.addToParents(par_c);
  par_c.addToDaughters(par_c_c);

  edm4hep::MutableMCParticle par_c_d = edm4hep::MutableMCParticle();
  par_c_d.setPDG(22);
  par_c_d.setCharge(0.0);
  par_c_d.setMass(0.0);
  par_c_d.setGeneratorStatus(0);
  par_c_d.addToParents(par_c);
  par_c.addToDaughters(par_c_d);

  auto mc_par_coll = std::make_unique<edm4hep::MCParticleCollection>();
  mc_par_coll->push_back(par_a);
  mc_par_coll->push_back(par_b);
  mc_par_coll->push_back(par_c);
  mc_par_coll->push_back(par_c_c);
  mc_par_coll->push_back(par_c_d);

  // define contributions
  //   - par A: 2.5 GeV --> hit A
  //   - par A: 0.5 GeV --> hit b
  //   - par B: 2.0 GeV --> hit b
  //   - par C: 0.5 GeV --> hit c (via decay photon)
  //   - par C: 0.5 GeV --> hit d (via decay photon)
  auto calo_hit_contrib_coll = std::make_unique<edm4hep::CaloHitContributionCollection>();
  auto contrib_a_a           = calo_hit_contrib_coll->create(11, 2.5);
  auto contrib_a_b           = calo_hit_contrib_coll->create(11, 0.5);
  auto contrib_b             = calo_hit_contrib_coll->create(22, 2.0);
  auto contrib_c_c           = calo_hit_contrib_coll->create(22, 0.5);
  auto contrib_c_d           = calo_hit_contrib_coll->create(22, 0.5);
  contrib_a_a.setParticle(par_a);
  contrib_a_b.setParticle(par_a);
  contrib_b.setParticle(par_b);
  contrib_c_c.setParticle(par_c_c);
  contrib_c_d.setParticle(par_c_d);

  auto sim_calo_hit_coll = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
  auto sim_hit_a         = sim_calo_hit_coll->create(0, 2.5);
  auto sim_hit_b         = sim_calo_hit_coll->create(1, 2.5);
  auto sim_hit_c         = sim_calo_hit_coll->create(2, 0.5);
  auto sim_hit_d         = sim_calo_hit_coll->create(3, 0.5);
  sim_hit_a.addToContributions(contrib_a_a);
  sim_hit_b.addToContributions(contrib_a_b);
  sim_hit_b.addToContributions(contrib_b);
  sim_hit_c.addToContributions(contrib_c_c);
  sim_hit_d.addToContributions(contrib_c_d);

  auto raw_calo_hit_coll = std::make_unique<edm4hep::RawCalorimeterHitCollection>();
  auto raw_hit_a         = raw_calo_hit_coll->create(0, 250);
  auto raw_hit_b         = raw_calo_hit_coll->create(1, 250);
  auto raw_hit_c         = raw_calo_hit_coll->create(2, 50);
  auto raw_hit_d         = raw_calo_hit_coll->create(3, 50);

  auto mc_rec_hit_link_coll = std::make_unique<edm4eic::MCRecoCalorimeterHitLinkCollection>();
  auto mc_rec_hit_link_a    = mc_rec_hit_link_coll->create();
  auto mc_rec_hit_link_b    = mc_rec_hit_link_coll->create();
  auto mc_rec_hit_link_c    = mc_rec_hit_link_coll->create();
  auto mc_rec_hit_link_d    = mc_rec_hit_link_coll->create();
  mc_rec_hit_link_a.setFrom(raw_hit_a);
  mc_rec_hit_link_a.setTo(sim_hit_a);
  mc_rec_hit_link_b.setFrom(raw_hit_b);
  mc_rec_hit_link_b.setTo(sim_hit_b);
  mc_rec_hit_link_c.setFrom(raw_hit_c);
  mc_rec_hit_link_c.setTo(sim_hit_c);
  mc_rec_hit_link_d.setFrom(raw_hit_d);
  mc_rec_hit_link_d.setTo(sim_hit_d);

  auto rec_calo_hit_coll = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto rec_hit_a         = rec_calo_hit_coll->create(0, 2.5);
  auto rec_hit_b         = rec_calo_hit_coll->create(1, 2.5);
  auto rec_hit_c         = rec_calo_hit_coll->create(2, 0.5);
  auto rec_hit_d         = rec_calo_hit_coll->create(3, 0.5);
  rec_hit_a.setRawHit(raw_hit_a);
  rec_hit_b.setRawHit(raw_hit_b);
  rec_hit_c.setRawHit(raw_hit_c);
  rec_hit_d.setRawHit(raw_hit_d);

  // cluster rec hits based on truth info: should produce 3 clusters
  //   - clust A = {hit_a, hit_b}
  //   - clust B = {hit_b}
  //   - clust C = {hit_c, hit_d}
  auto truth_clust_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
  algo_clustering.process({rec_calo_hit_coll.get(), mc_rec_hit_link_coll.get()},
                          {truth_clust_coll.get()});

  SECTION("algorithm produces correct number of outputs") {
    REQUIRE(truth_clust_coll->size() == 3);
  }

  SECTION("hits are assigned to the correct clusters") {
    const std::set clust_a{0, 1};
    const std::set clust_b{1};
    const std::set clust_c{2, 3};
    for (const auto& clust : *truth_clust_coll) {
      for (const auto& hit : clust.getHits()) {
        const auto cell_id = hit.getCellID();
        switch (cell_id) {
        case 0:
          REQUIRE(clust_a.contains(cell_id));
          break;
        case 1:
          REQUIRE(clust_a.contains(cell_id));
          REQUIRE(clust_b.contains(cell_id));
          break;
        case 2:
          REQUIRE(clust_c.contains(cell_id));
          break;
        case 3:
          REQUIRE(clust_c.contains(cell_id));
          break;
        default:
          FAIL("Unknown cell ID encountered");
          break;
        }
      }
    }
  }

  // weights are ratio of sim energy contributed by a particle to hit over
  // total sim energy of hit
  //   clust A --> {weight_a = 1.0, weight_b = 0.2}
  //   clust B --> {weight_b = 0.8}
  //   clust C --> {weight_c = 1.0, weight_d = 1.0}
  SECTION("hits are assigned the correct weights") {
    for (const auto& clust : *truth_clust_coll) {
      for (std::size_t ihit = 0; const auto& hit : clust.getHits()) {
        const auto cell_id = hit.getCellID();
        const float cell_w = clust.getWeights()[ihit];
        switch (cell_id) {
        case 0:
          REQUIRE(std::abs(cell_w - 1.0) < std::numeric_limits<float>::epsilon());
          break;
        case 1:
          // if parent cluster has 2 hits, then in cluster A
          // otherwise in cluster B
          if (clust.getHits().size() == 2) {
            REQUIRE(std::abs(cell_w - 0.2) < std::numeric_limits<float>::epsilon());
          } else {
            REQUIRE(std::abs(cell_w - 0.8) < std::numeric_limits<float>::epsilon());
          }
          break;
        case 2:
          REQUIRE(std::abs(cell_w - 1.0) < std::numeric_limits<float>::epsilon());
          break;
        case 3:
          REQUIRE(std::abs(cell_w - 1.0) < std::numeric_limits<float>::epsilon());
          break;
        default:
          FAIL("Unknown cell ID encountered");
          break;
        }
        ++ihit;
      }
    }
  }
}
