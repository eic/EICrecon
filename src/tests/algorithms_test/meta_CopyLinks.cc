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

  SECTION("algorithm correctly handles copying between the same type (subsets)") {

    eicrecon::CopyLinks<edm4eic::ReconstructedParticle, edm4eic::MCRecoParticleLink>
        algo_copy_subset("CopyLinksSubset");
    algo_copy_subset.level(algorithms::LogLevel::kDebug);
    algo_copy_subset.init();

    auto empty_sub_coll = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto empty_rec_coll = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto empty_sub_link = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto empty_rec_link = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    algo_copy_subset.process({empty_sub_coll.get(), empty_rec_coll.get(),
                              empty_sub_link.get()},
                             {empty_rec_link.get()});

    // make sure empty inputs produces empty outputs
    REQUIRE(empty_rec_link->size() == 0);

    // create rec particle collections
    auto sub_coll_a = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_1      = sub_coll_a->create();
    auto sub_2      = sub_coll_a->create();
    auto sub_3      = sub_coll_a->create();

    auto sub_coll_b = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_4      = sub_coll_b->create();
    auto sub_5      = sub_coll_b->create();
    auto sub_6      = sub_coll_b->create();

    // create sim particles to link to
    auto sim_coll = std::make_unique<edm4hep::MCParticleCollection>();
    auto sim_1    = sim_coll->create();
    auto sim_2    = sim_coll->create();
    auto sim_3    = sim_coll->create();
    auto sim_4    = sim_coll->create();
    auto sim_5    = sim_coll->create();
    auto sim_6    = sim_coll->create();

    // link particles
    //   - rec 1 <--- {sim 1, sim 2, sim 3}
    //   - rec 2 <--- {sim 4}
    //   - sim 5 ---> {rec 3, rec 4, rec 5}
    //   - sim 6 ---> {rec 6}
    auto sub_link_a = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto sub_link_1 = sub_link_a->create();
    auto sub_link_2 = sub_link_a->create();
    auto sub_link_3 = sub_link_a->create();
    auto sub_link_4 = sub_link_a->create();
    auto sub_link_5 = sub_link_a->create();
    sub_link_1.setFrom(sub_1);
    sub_link_1.setTo(sim_1);
    sub_link_2.setFrom(sub_1);
    sub_link_2.setTo(sim_2);
    sub_link_3.setFrom(sub_1);
    sub_link_3.setTo(sim_3);
    sub_link_4.setFrom(sub_2);
    sub_link_4.setTo(sim_4);
    sub_link_5.setFrom(sub_3);
    sub_link_5.setTo(sim_5);

    auto sub_link_b = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto sub_link_6 = sub_link_b->create();
    auto sub_link_7 = sub_link_b->create();
    auto sub_link_8 = sub_link_b->create();
    sub_link_6.setFrom(sub_4);
    sub_link_6.setTo(sim_5);
    sub_link_7.setFrom(sub_5);
    sub_link_7.setTo(sim_5);
    sub_link_8.setFrom(sub_6);
    sub_link_8.setTo(sim_6);

    // collect particles/links, clone particles
    auto rec_coll = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_coll = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_link = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();

    std::vector<gsl::not_null<const edm4eic::ReconstructedParticleCollection*>> collected_parts = {
        sub_coll_a.get(), sub_coll_b.get()};
    std::vector<gsl::not_null<const edm4eic::MCRecoParticleLinkCollection*>> collected_links = {
        sub_link_a.get(), sub_link_b.get()};

    eicrecon::CollectionCollector<edm4eic::ReconstructedParticleCollection> algo_collect_parts(
        "CollectParts");
    algo_collect_parts.level(algorithms::LogLevel::kDebug);
    algo_collect_parts.init();
    algo_collect_parts.process({collected_parts}, {sub_coll.get()});

    eicrecon::CollectionCollector<edm4eic::MCRecoParticleLinkCollection> algo_collect_links(
        "CollectLinks");
    algo_collect_links.level(algorithms::LogLevel::kDebug);
    algo_collect_links.init();
    algo_collect_links.process({collected_links}, {sub_link.get()});

    eicrecon::Cloner<edm4eic::ReconstructedParticle> algo_clone_parts("CloneParts");
    algo_clone_parts.level(algorithms::LogLevel::kDebug);
    algo_clone_parts.init();
    algo_clone_parts.process({sub_coll.get()}, {rec_coll.get()});

    auto rec_link   = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    auto rec_link_1 = rec_link->create();
    auto rec_link_2 = rec_link->create();
    auto rec_link_3 = rec_link->create();
    auto rec_link_4 = rec_link->create();
    auto rec_link_5 = rec_link->create();
    auto rec_link_6 = rec_link->create();
    auto rec_link_7 = rec_link->create();
    auto rec_link_8 = rec_link->create();
    rec_link_1.setFrom(rec_coll->at(0)); // sub_1
    rec_link_1.setTo(sim_1);
    rec_link_2.setFrom(rec_coll->at(0)); // sub_1
    rec_link_2.setTo(sim_2);
    rec_link_3.setFrom(rec_coll->at(0)); // sub_1
    rec_link_3.setTo(sim_3);
    rec_link_4.setFrom(rec_coll->at(1)); // sub_2
    rec_link_4.setTo(sim_4);
    rec_link_5.setFrom(rec_coll->at(2)); // sub_3
    rec_link_5.setTo(sim_5);
    rec_link_6.setFrom(rec_coll->at(3)); // sub_4
    rec_link_6.setTo(sim_5);
    rec_link_7.setFrom(rec_coll->at(4)); // sub_5
    rec_link_7.setTo(sim_5);
    rec_link_8.setFrom(rec_coll->at(5)); // sub_6
    rec_link_8.setTo(sim_6);

    // copy links over to full collection
    auto rec_algo = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
    algo_copy_subset.process(
        {sub_coll.get(), rec_coll.get(), sub_link.get()},
        {rec_algo.get()});

    // make sure algorithm produced the right number of links
    REQUIRE(rec_link->size() == rec_algo->size());

    // make sure links have been propagated correctly
    for (std::size_t idx = 0; idx < rec_link->size(); ++idx) {
      REQUIRE((*rec_link)[idx].getFrom() == (*rec_algo)[idx].getFrom());
      REQUIRE((*rec_link)[idx].getTo() == (*rec_algo)[idx].getTo());
    }
  }

  SECTION("algorithm correctly handles copying between two types (promotion)") {

    eicrecon::CopyLinks<edm4eic::ProtoCluster, edm4eic::TrackProtoClusterLink, edm4eic::Cluster,
                        edm4eic::TrackClusterLink>
        algo_promote_links("PromoteLinks");
    algo_promote_links.level(algorithms::LogLevel::kDebug);
    algo_promote_links.init();

    auto empty_proto_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
    auto empty_clust_coll = std::make_unique<edm4eic::ClusterCollection>();
    auto empty_proto_link = std::make_unique<edm4eic::TrackProtoClusterLinkCollection>();
    auto empty_clust_link = std::make_unique<edm4eic::TrackClusterLinkCollection>();
    algo_promote_links.process(
        {empty_proto_coll.get(), empty_clust_coll.get(), empty_proto_link.get()},
        {empty_clust_link.get()});

    // make sure empty inputs produces empty outputs
    REQUIRE(empty_clust_link->size() == 0);

    // create proto/clusters
    auto proto_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
    auto proto_1    = proto_coll->create();
    auto proto_2    = proto_coll->create();
    auto proto_3    = proto_coll->create();
    auto proto_4    = proto_coll->create();
    auto proto_5    = proto_coll->create();
    auto proto_6    = proto_coll->create();

    auto clust_coll = std::make_unique<edm4eic::ClusterCollection>();
    auto clust_1    = clust_coll->create();
    auto clust_2    = clust_coll->create();
    auto clust_3    = clust_coll->create();
    auto clust_4    = clust_coll->create();
    auto clust_5    = clust_coll->create();
    auto clust_6    = clust_coll->create();

    // create tracks to link to
    auto track_coll = std::make_unique<edm4eic::TrackCollection>();
    auto track_1    = track_coll->create();
    auto track_2    = track_coll->create();
    auto track_3    = track_coll->create();
    auto track_4    = track_coll->create();
    auto track_5    = track_coll->create();
    auto track_6    = track_coll->create();

    // link proto/clusters to tracks
    //   - proto 1 <--- {track 1, track 2, track 3}
    //   - proto 2 <--- {track 4}
    //   - track 5 ---> {proto 3, proto 4, proto 5}
    //   - track 6 ---> {proto 6}
    auto proto_link   = std::make_unique<edm4eic::TrackProtoClusterLinkCollection>();
    auto proto_link_1 = proto_link->create();
    auto proto_link_2 = proto_link->create();
    auto proto_link_3 = proto_link->create();
    auto proto_link_4 = proto_link->create();
    auto proto_link_5 = proto_link->create();
    auto proto_link_6 = proto_link->create();
    auto proto_link_7 = proto_link->create();
    auto proto_link_8 = proto_link->create();
    proto_link_1.setFrom(track_1);
    proto_link_1.setTo(proto_1);
    proto_link_2.setFrom(track_2);
    proto_link_2.setTo(proto_1);
    proto_link_3.setFrom(track_3);
    proto_link_3.setTo(proto_1);
    proto_link_4.setFrom(track_4);
    proto_link_4.setTo(proto_2);
    proto_link_5.setFrom(track_5);
    proto_link_5.setTo(proto_3);
    proto_link_6.setFrom(track_5);
    proto_link_6.setTo(proto_4);
    proto_link_7.setFrom(track_5);
    proto_link_7.setTo(proto_5);
    proto_link_8.setFrom(track_6);
    proto_link_8.setTo(proto_6);

    auto clust_link   = std::make_unique<edm4eic::TrackClusterLinkCollection>();
    auto clust_link_1 = clust_link->create();
    auto clust_link_2 = clust_link->create();
    auto clust_link_3 = clust_link->create();
    auto clust_link_4 = clust_link->create();
    auto clust_link_5 = clust_link->create();
    auto clust_link_6 = clust_link->create();
    auto clust_link_7 = clust_link->create();
    auto clust_link_8 = clust_link->create();
    clust_link_1.setTo(track_1);
    clust_link_1.setFrom(clust_1);
    clust_link_2.setTo(track_2);
    clust_link_2.setFrom(clust_1);
    clust_link_3.setTo(track_3);
    clust_link_3.setFrom(clust_1);
    clust_link_4.setTo(track_4);
    clust_link_4.setFrom(clust_2);
    clust_link_5.setTo(track_5);
    clust_link_5.setFrom(clust_3);
    clust_link_6.setTo(track_5);
    clust_link_6.setFrom(clust_4);
    clust_link_7.setTo(track_5);
    clust_link_7.setFrom(clust_5);
    clust_link_8.setTo(track_6);
    clust_link_8.setFrom(clust_6);

    // copy protocluster links to clusters
    auto clust_algo = std::make_unique<edm4eic::TrackClusterLinkCollection>();
    algo_promote_links.process(
        {proto_coll.get(), clust_coll.get(), proto_link.get()},
        {clust_algo.get()});

    // make sure algorithm produced the right number of links
    REQUIRE(clust_link->size() == clust_algo->size());

    // make sure links have been propagated correctly
    for (std::size_t idx = 0; idx < clust_link->size(); ++idx) {
      REQUIRE((*clust_link)[idx].getFrom() == (*clust_algo)[idx].getFrom());
      REQUIRE((*clust_link)[idx].getTo() == (*clust_algo)[idx].getTo());
    }
  }
}
