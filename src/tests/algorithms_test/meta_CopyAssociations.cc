// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackProtoClusterMatchCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/meta/Cloner.h"
#include "algorithms/meta/CollectionCollector.h"
#include "algorithms/meta/CopyAssociations.h"
#include "algorithms/meta/CopyAssociationsConfig.h"

TEST_CASE("the CopyAssociations algorithm runs", "[CopyAssociations]") {

  SECTION("algorithm correctly handles copying between the same type (subsets)") {

    auto get_source_rec = [](const edm4eic::MCRecoParticleAssociation& association)
        -> edm4eic::ReconstructedParticle { return association.getRec(); };
    auto get_source_sim =
        [](const edm4eic::MCRecoParticleAssociation& association) -> edm4hep::MCParticle {
      return association.getSim();
    };
    auto set_target_rec = [](const edm4eic::ReconstructedParticle& target,
                             edm4eic::MutableMCRecoParticleAssociation& association) -> void {
      association.setRec(target);
    };
    auto set_target_sim = [](const edm4hep::MCParticle& associate,
                             edm4eic::MutableMCRecoParticleAssociation& association) -> void {
      association.setSim(associate);
    };

    eicrecon::CopyAssociationsConfig<edm4eic::ReconstructedParticle,
                                     edm4eic::MCRecoParticleAssociation, edm4hep::MCParticle>
        cfg_copy_subset;
    cfg_copy_subset.getSourceFrom = get_source_rec;
    cfg_copy_subset.getSourceTo   = get_source_sim;
    cfg_copy_subset.setTargetFrom = set_target_rec;
    cfg_copy_subset.setTargetTo   = set_target_sim;

    eicrecon::CopyAssociations<edm4eic::ReconstructedParticle, edm4eic::MCRecoParticleAssociation,
                               edm4hep::MCParticle>
        algo_copy_subset("CopyAssociationsSubset");
    algo_copy_subset.applyConfig(cfg_copy_subset);
    algo_copy_subset.level(algorithms::LogLevel::kDebug);
    algo_copy_subset.init();

    auto empty_sub_coll  = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto empty_rec_coll  = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto empty_sub_assoc = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto empty_rec_assoc = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    algo_copy_subset.process({empty_sub_coll.get(), empty_rec_coll.get(), empty_sub_assoc.get()},
                             {empty_rec_assoc.get()});

    // make sure empty inputs produces empty outputs
    REQUIRE(empty_rec_assoc->size() == 0);

    // create rec particle collections
    auto sub_coll_a = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_1      = sub_coll_a->create();
    auto sub_2      = sub_coll_a->create();
    auto sub_3      = sub_coll_a->create();

    auto sub_coll_b = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_4      = sub_coll_b->create();
    auto sub_5      = sub_coll_b->create();
    auto sub_6      = sub_coll_b->create();

    // create sim particles to associate to
    auto sim_coll = std::make_unique<edm4hep::MCParticleCollection>();
    auto sim_1    = sim_coll->create();
    auto sim_2    = sim_coll->create();
    auto sim_3    = sim_coll->create();
    auto sim_4    = sim_coll->create();
    auto sim_5    = sim_coll->create();
    auto sim_6    = sim_coll->create();

    // associate particles
    //   - rec 1 <--- {sim 1, sim 2, sim 3}
    //   - rec 2 <--- {sim 4}
    //   - sim 5 ---> {rec 3, rec 4, rec 5}
    //   - sim 6 ---> {rec 6}
    auto sub_assoc_a = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto sub_assoc_1 = sub_assoc_a->create();
    auto sub_assoc_2 = sub_assoc_a->create();
    auto sub_assoc_3 = sub_assoc_a->create();
    auto sub_assoc_4 = sub_assoc_a->create();
    auto sub_assoc_5 = sub_assoc_a->create();
    sub_assoc_1.setRec(sub_1);
    sub_assoc_1.setSim(sim_1);
    sub_assoc_2.setRec(sub_1);
    sub_assoc_2.setSim(sim_2);
    sub_assoc_3.setRec(sub_1);
    sub_assoc_3.setSim(sim_3);
    sub_assoc_4.setRec(sub_2);
    sub_assoc_4.setSim(sim_4);
    sub_assoc_5.setRec(sub_3);
    sub_assoc_5.setSim(sim_5);

    auto sub_assoc_b = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto sub_assoc_6 = sub_assoc_b->create();
    auto sub_assoc_7 = sub_assoc_b->create();
    auto sub_assoc_8 = sub_assoc_b->create();
    sub_assoc_6.setRec(sub_4);
    sub_assoc_6.setSim(sim_5);
    sub_assoc_7.setRec(sub_5);
    sub_assoc_7.setSim(sim_5);
    sub_assoc_8.setRec(sub_6);
    sub_assoc_8.setSim(sim_6);

    // collect particles/assocs, clone particles
    auto rec_coll  = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_coll  = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto sub_assoc = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();

    std::vector<gsl::not_null<const edm4eic::ReconstructedParticleCollection*>> collected_parts = {
        sub_coll_a.get(), sub_coll_b.get()};
    std::vector<gsl::not_null<const edm4eic::MCRecoParticleAssociationCollection*>>
        collected_assocs = {sub_assoc_a.get(), sub_assoc_b.get()};

    eicrecon::CollectionCollector<edm4eic::ReconstructedParticleCollection> algo_collect_parts(
        "CollectParts");
    algo_collect_parts.level(algorithms::LogLevel::kDebug);
    algo_collect_parts.init();
    algo_collect_parts.process({collected_parts}, {sub_coll.get()});

    eicrecon::CollectionCollector<edm4eic::MCRecoParticleAssociationCollection> algo_collect_assocs(
        "CollectAssociations");
    algo_collect_assocs.level(algorithms::LogLevel::kDebug);
    algo_collect_assocs.init();
    algo_collect_assocs.process({collected_assocs}, {sub_assoc.get()});

    eicrecon::Cloner<edm4eic::ReconstructedParticle> algo_clone_parts("CloneParts");
    algo_clone_parts.level(algorithms::LogLevel::kDebug);
    algo_clone_parts.init();
    algo_clone_parts.process({sub_coll.get()}, {rec_coll.get()});

    auto rec_assoc   = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto rec_assoc_1 = rec_assoc->create();
    auto rec_assoc_2 = rec_assoc->create();
    auto rec_assoc_3 = rec_assoc->create();
    auto rec_assoc_4 = rec_assoc->create();
    auto rec_assoc_5 = rec_assoc->create();
    auto rec_assoc_6 = rec_assoc->create();
    auto rec_assoc_7 = rec_assoc->create();
    auto rec_assoc_8 = rec_assoc->create();
    rec_assoc_1.setRec(rec_coll->at(0)); // sub_1
    rec_assoc_1.setSim(sim_1);
    rec_assoc_2.setRec(rec_coll->at(0)); // sub_1
    rec_assoc_2.setSim(sim_2);
    rec_assoc_3.setRec(rec_coll->at(0)); // sub_1
    rec_assoc_3.setSim(sim_3);
    rec_assoc_4.setRec(rec_coll->at(1)); // sub_2
    rec_assoc_4.setSim(sim_4);
    rec_assoc_5.setRec(rec_coll->at(2)); // sub_3
    rec_assoc_5.setSim(sim_5);
    rec_assoc_6.setRec(rec_coll->at(3)); // sub_4
    rec_assoc_6.setSim(sim_5);
    rec_assoc_7.setRec(rec_coll->at(4)); // sub_5
    rec_assoc_7.setSim(sim_5);
    rec_assoc_8.setRec(rec_coll->at(5)); // sub_6
    rec_assoc_8.setSim(sim_6);

    // copy assocs over to full collection
    auto rec_algo = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    algo_copy_subset.process({sub_coll.get(), rec_coll.get(), sub_assoc.get()}, {rec_algo.get()});

    // make sure algorithm produced the right number of associations
    REQUIRE(rec_assoc->size() == rec_algo->size());

    // make sure associations have been propagated correctly
    for (std::size_t idx = 0; idx < rec_assoc->size(); ++idx) {
      REQUIRE((*rec_assoc)[idx].getRec() == (*rec_algo)[idx].getRec());
      REQUIRE((*rec_assoc)[idx].getSim() == (*rec_algo)[idx].getSim());
    }
  }

  SECTION("algorithm correctly handles copying between two types (promotion)") {

    auto get_source_proto =
        [](const edm4eic::TrackProtoClusterMatch& match) -> edm4eic::ProtoCluster {
      return match.getTo();
    };
    auto get_source_track = [](const edm4eic::TrackProtoClusterMatch& match) -> edm4eic::Track {
      return match.getFrom();
    };
    auto set_target_clust = [](const edm4eic::Cluster& target,
                               edm4eic::MutableTrackClusterMatch& match) -> void {
      match.setCluster(target);
    };
    auto set_target_track = [](const edm4eic::Track& matched,
                               edm4eic::MutableTrackClusterMatch& match) -> void {
      match.setTrack(matched);
    };

    eicrecon::CopyAssociationsConfig<edm4eic::ProtoCluster, edm4eic::TrackProtoClusterMatch,
                                     edm4eic::Track, edm4eic::Cluster, edm4eic::TrackClusterMatch>
        cfg_promote_assocs;
    cfg_promote_assocs.getSourceFrom = get_source_proto;
    cfg_promote_assocs.getSourceTo   = get_source_track;
    cfg_promote_assocs.setTargetFrom = set_target_clust;
    cfg_promote_assocs.setTargetTo   = set_target_track;

    eicrecon::CopyAssociations<edm4eic::ProtoCluster, edm4eic::TrackProtoClusterMatch,
                               edm4eic::Track, edm4eic::Cluster, edm4eic::TrackClusterMatch>
        algo_promote_assocs("PromoteAssociations");
    algo_promote_assocs.applyConfig(cfg_promote_assocs);
    algo_promote_assocs.level(algorithms::LogLevel::kDebug);
    algo_promote_assocs.init();

    auto empty_proto_coll  = std::make_unique<edm4eic::ProtoClusterCollection>();
    auto empty_clust_coll  = std::make_unique<edm4eic::ClusterCollection>();
    auto empty_proto_assoc = std::make_unique<edm4eic::TrackProtoClusterMatchCollection>();
    auto empty_clust_assoc = std::make_unique<edm4eic::TrackClusterMatchCollection>();
    algo_promote_assocs.process(
        {empty_proto_coll.get(), empty_clust_coll.get(), empty_proto_assoc.get()},
        {empty_clust_assoc.get()});

    // make sure empty inputs produces empty outputs
    REQUIRE(empty_clust_assoc->size() == 0);

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

    // create tracks to assoc to
    auto track_coll = std::make_unique<edm4eic::TrackCollection>();
    auto track_1    = track_coll->create();
    auto track_2    = track_coll->create();
    auto track_3    = track_coll->create();
    auto track_4    = track_coll->create();
    auto track_5    = track_coll->create();
    auto track_6    = track_coll->create();

    // assoc proto/clusters to tracks
    //   - proto 1 <--- {track 1, track 2, track 3}
    //   - proto 2 <--- {track 4}
    //   - track 5 ---> {proto 3, proto 4, proto 5}
    //   - track 6 ---> {proto 6}
    auto proto_assoc   = std::make_unique<edm4eic::TrackProtoClusterMatchCollection>();
    auto proto_assoc_1 = proto_assoc->create();
    auto proto_assoc_2 = proto_assoc->create();
    auto proto_assoc_3 = proto_assoc->create();
    auto proto_assoc_4 = proto_assoc->create();
    auto proto_assoc_5 = proto_assoc->create();
    auto proto_assoc_6 = proto_assoc->create();
    auto proto_assoc_7 = proto_assoc->create();
    auto proto_assoc_8 = proto_assoc->create();
    proto_assoc_1.setFrom(track_1);
    proto_assoc_1.setTo(proto_1);
    proto_assoc_2.setFrom(track_2);
    proto_assoc_2.setTo(proto_1);
    proto_assoc_3.setFrom(track_3);
    proto_assoc_3.setTo(proto_1);
    proto_assoc_4.setFrom(track_4);
    proto_assoc_4.setTo(proto_2);
    proto_assoc_5.setFrom(track_5);
    proto_assoc_5.setTo(proto_3);
    proto_assoc_6.setFrom(track_5);
    proto_assoc_6.setTo(proto_4);
    proto_assoc_7.setFrom(track_5);
    proto_assoc_7.setTo(proto_5);
    proto_assoc_8.setFrom(track_6);
    proto_assoc_8.setTo(proto_6);

    auto clust_assoc   = std::make_unique<edm4eic::TrackClusterMatchCollection>();
    auto clust_assoc_1 = clust_assoc->create();
    auto clust_assoc_2 = clust_assoc->create();
    auto clust_assoc_3 = clust_assoc->create();
    auto clust_assoc_4 = clust_assoc->create();
    auto clust_assoc_5 = clust_assoc->create();
    auto clust_assoc_6 = clust_assoc->create();
    auto clust_assoc_7 = clust_assoc->create();
    auto clust_assoc_8 = clust_assoc->create();
    clust_assoc_1.setTrack(track_1);
    clust_assoc_1.setCluster(clust_1);
    clust_assoc_2.setTrack(track_2);
    clust_assoc_2.setCluster(clust_1);
    clust_assoc_3.setTrack(track_3);
    clust_assoc_3.setCluster(clust_1);
    clust_assoc_4.setTrack(track_4);
    clust_assoc_4.setCluster(clust_2);
    clust_assoc_5.setTrack(track_5);
    clust_assoc_5.setCluster(clust_3);
    clust_assoc_6.setTrack(track_5);
    clust_assoc_6.setCluster(clust_4);
    clust_assoc_7.setTrack(track_5);
    clust_assoc_7.setCluster(clust_5);
    clust_assoc_8.setTrack(track_6);
    clust_assoc_8.setCluster(clust_6);

    // copy protocluster assocs to clusters
    auto clust_algo = std::make_unique<edm4eic::TrackClusterMatchCollection>();
    algo_promote_assocs.process({proto_coll.get(), clust_coll.get(), proto_assoc.get()},
                                {clust_algo.get()});

    // make sure algorithm produced the right number of assocs
    REQUIRE(clust_assoc->size() == clust_algo->size());

    // make sure assocs have been propagated correctly
    for (std::size_t idx = 0; idx < clust_assoc->size(); ++idx) {
      REQUIRE((*clust_assoc)[idx].getCluster() == (*clust_algo)[idx].getCluster());
      REQUIRE((*clust_assoc)[idx].getTrack() == (*clust_algo)[idx].getTrack());
    }
  }
}
