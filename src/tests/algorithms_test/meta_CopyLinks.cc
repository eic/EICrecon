// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterLinkCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackProtoClusterLinkCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <podio/LinkNavigator.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <cstddef>
#include <deque>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "algorithms/meta/Cloner.h"
#include "algorithms/meta/CollectionCollector.h"
#include "algorithms/meta/CopyLinks.h"

TEST_CASE("the CopyLinks algorithm runs", "[CopyLinks]") {

  SECTION("algorithm correctly handles subsets") {

    eicrecon::CopyLinks<edm4eic::ReconstructedParticle, edm4eic::MCRecoParticleLink>
        algo_copy_subset("CopyLinksSubset");
    algo_copy_subset.level(algorithms::LogLevel::kDebug);
    algo_copy_subset.init();

    auto empty_rec_part_sub_coll    = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto empty_rec_part_coll        = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto empty_mc_rec_link_sub_coll = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto empty_mc_rec_link_coll     = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    algo_copy_subset.process({empty_rec_part_sub_coll.get(), empty_rec_part_coll.get(),
                              empty_mc_rec_link_sub_coll.get()},
                             {empty_mc_rec_link_coll.get()});

    // make sure empty inputs produces empty outputs
    REQUIRE(empty_mc_rec_link_coll->size() == 0);

    // create reco particle collections to collect
    auto rec_part_coll_a = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto rec_part_1      = rec_part_coll_a->create();
    auto rec_part_2      = rec_part_coll_a->create();
    auto rec_part_3      = rec_part_coll_a->create();

    auto rec_part_coll_b = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto rec_part_4      = rec_part_coll_b->create();
    auto rec_part_5      = rec_part_coll_b->create();
    auto rec_part_6      = rec_part_coll_b->create();

    // create mc particles to link to
    auto mc_part_coll = std::make_unique<edm4hep::MCParticleCollection>();
    auto mc_part_1    = mc_part_coll->create();
    auto mc_part_2    = mc_part_coll->create();
    auto mc_part_3    = mc_part_coll->create();
    auto mc_part_4    = mc_part_coll->create();
    auto mc_part_5    = mc_part_coll->create();
    auto mc_part_6    = mc_part_coll->create();

    // link particles for set A
    //   - rec 1 <--- {mc 1, mc 2, mc 3}
    //   - rec 2 <--- {mc 4}
    //   - rec 3 <--- {mc 5}
    auto mc_rec_link_coll_a = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto mc_rec_link_1      = mc_rec_link_coll_a->create();
    auto mc_rec_link_2      = mc_rec_link_coll_a->create();
    auto mc_rec_link_3      = mc_rec_link_coll_a->create();
    auto mc_rec_link_4      = mc_rec_link_coll_a->create();
    auto mc_rec_link_5      = mc_rec_link_coll_a->create();
    mc_rec_link_1.setFrom(rec_part_1);
    mc_rec_link_1.setTo(mc_part_1);
    mc_rec_link_2.setFrom(rec_part_1);
    mc_rec_link_2.setTo(mc_part_2);
    mc_rec_link_3.setFrom(rec_part_1);
    mc_rec_link_3.setTo(mc_part_3);
    mc_rec_link_4.setFrom(rec_part_2);
    mc_rec_link_4.setTo(mc_part_4);
    mc_rec_link_5.setFrom(rec_part_3);
    mc_rec_link_5.setTo(mc_part_5);

    // link particles for set B
    //   - mc 5 --> {rec 4, rec 5}
    //   - mc 6 --> {rec 6}
    auto mc_rec_link_coll_b = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto mc_rec_link_6      = mc_rec_link_coll_b->create();
    auto mc_rec_link_7      = mc_rec_link_coll_b->create();
    auto mc_rec_link_8      = mc_rec_link_coll_b->create();
    mc_rec_link_6.setFrom(rec_part_4);
    mc_rec_link_6.setTo(mc_part_5);
    mc_rec_link_7.setFrom(rec_part_5);
    mc_rec_link_7.setTo(mc_part_5);
    mc_rec_link_8.setFrom(rec_part_6);
    mc_rec_link_8.setTo(mc_part_6);

    // collect particles/links
    auto rec_part_sub_coll    = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto mc_rec_link_sub_coll = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();

    std::vector<gsl::not_null<const edm4eic::ReconstructedParticleCollection*>> collected_parts = {
        rec_part_coll_a.get(), rec_part_coll_b.get()};
    std::vector<gsl::not_null<const edm4eic::MCRecoParticleLinkCollection*>> collected_links = {
        mc_rec_link_coll_a.get(), mc_rec_link_coll_b.get()};

    eicrecon::CollectionCollector<edm4eic::ReconstructedParticleCollection> algo_collect_parts(
        "CollectParts");
    algo_collect_parts.level(algorithms::LogLevel::kDebug);
    algo_collect_parts.init();
    algo_collect_parts.process({collected_parts}, {rec_part_sub_coll.get()});

    eicrecon::CollectionCollector<edm4eic::MCRecoParticleLinkCollection> algo_collect_links(
        "CollectLinks");
    algo_collect_links.level(algorithms::LogLevel::kDebug);
    algo_collect_links.init();
    algo_collect_links.process({collected_links}, {mc_rec_link_sub_coll.get()});

    // clone particles from subset to full collection
    auto rec_part_coll    = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto mc_rec_link_coll = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();

    eicrecon::Cloner<edm4eic::ReconstructedParticle> algo_clone_parts("CloneParts");
    algo_clone_parts.level(algorithms::LogLevel::kDebug);
    algo_clone_parts.init();
    algo_clone_parts.process({rec_part_sub_coll.get()}, {rec_part_coll.get()});

    // and now copy links over to full collection
    algo_copy_subset.process(
        {rec_part_sub_coll.get(), rec_part_coll.get(), mc_rec_link_sub_coll.get()},
        {mc_rec_link_coll.get()});

    // make sure algorithm produced the right number of links
    REQUIRE(mc_rec_link_coll->size() == mc_rec_link_sub_coll->size());

    // make sure links have been propagated correctly
    const auto mc_rec_sub_links = podio::LinkNavigator(*mc_rec_link_sub_coll);
    const auto mc_rec_links     = podio::LinkNavigator(*mc_rec_link_coll);
    for (std::size_t idx = 0; idx < rec_part_sub_coll->size(); ++idx) {

      // Nth subset and Nth full reco particle should have same no.
      // of linked mc particles
      const auto mc_rec_sub_linkees = mc_rec_sub_links.getLinked(rec_part_sub_coll->at(idx));
      const auto mc_rec_linkees     = mc_rec_links.getLinked(rec_part_coll->at(idx));
      REQUIRE(mc_rec_sub_linkees.size() == mc_rec_linkees.size());

      // and should be linked to same mc particles
      std::size_t n_mc_par_found = 0;
      for (const auto& [mc_sub_par, sub_weight] : mc_rec_sub_linkees) {
        for (const auto& [mc_par, weight] : mc_rec_linkees) {
          if (mc_sub_par == mc_par) {
            ++n_mc_par_found;
          }
        }
      }
      REQUIRE(n_mc_par_found == mc_rec_sub_linkees.size());
    }
  }

  SECTION("algorithm correctly handles promotions") {

    eicrecon::CopyLinks<edm4eic::ProtoCluster, edm4eic::TrackProtoClusterLink, edm4eic::Cluster,
                        edm4eic::TrackClusterLink>
        algo_promote_links("PromoteLinks");
    algo_promote_links.level(algorithms::LogLevel::kDebug);
    algo_promote_links.init();

    auto empty_proto_coll     = std::make_unique<edm4eic::ProtoClusterCollection>();
    auto empty_clust_coll     = std::make_unique<edm4eic::ClusterCollection>();
    auto empty_proto_trk_link = std::make_unique<edm4eic::TrackProtoClusterLinkCollection>();
    auto empty_clust_trk_link = std::make_unique<edm4eic::TrackClusterLinkCollection>();
    algo_promote_links.process(
        {empty_proto_coll.get(), empty_clust_coll.get(), empty_proto_trk_link.get()},
        {empty_clust_trk_link.get()});
    REQUIRE(empty_clust_trk_link->size() == 0);

    // TODO
  }
}
