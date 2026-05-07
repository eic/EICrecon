// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#endif
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackProtoClusterMatchCollection.h>
#include <edm4hep/Vector3f.h>
#include <memory>
#include <utility>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "algorithms/particle_flow/TrackProtoClusterMatchPromoter.h"

TEST_CASE("the TrackProtoClusterMatchPromoter algorithm runs", "[TrackProtoClusterMatchPromoter]") {

  eicrecon::TrackProtoClusterMatchPromoter algo_promote("trackProtoClusterMatchPromoter");
  algo_promote.level(algorithms::LogLevel::kDebug);
  algo_promote.init();

  SECTION("empty input produces empty output") {
    auto empty_proto_match_coll = std::make_unique<edm4eic::TrackProtoClusterMatchCollection>();
    auto empty_proto_coll       = std::make_unique<edm4eic::ProtoClusterCollection>();
    auto empty_clust_coll       = std::make_unique<edm4eic::ClusterCollection>();
    auto empty_clust_match_coll = std::make_unique<edm4eic::TrackClusterMatchCollection>();

    algo_promote.process({empty_proto_match_coll.get(), empty_proto_coll.get(), empty_clust_coll.get()}, {empty_clust_match_coll.get()});
    REQUIRE(empty_clust_match_coll->size() == 0);
  }

  auto hit_coll = std::make_unique<edm4eic::CalorimeterHitCollection>();
  auto hit1     = hit_coll->create(0, 1.0);
  auto hit2     = hit_coll->create(1, 1.5);
  auto hit3     = hit_coll->create(2, 2.0);
  auto hit4     = hit_coll->create(3, 2.0);
  auto hit5     = hit_coll->create(4, 2.0);
  auto hit6     = hit_coll->create(5, 3.0);

  // group hits into proto/clusters
  //   - proto/clust 1 = {hit 1, hit 2, hit 3}
  //   - proto/clust 2 = {hit 4, hit 5}
  //   - proto/clust 3 = {hit 6}
  auto proto_coll = std::make_unique<edm4eic::ProtoClusterCollection>();
  auto proto1     = proto_coll->create();
  auto proto2     = proto_coll->create();
  auto proto3     = proto_coll->create();
  proto1.addToHits(hit1);
  proto1.addToWeights(1.0);
  proto1.addToHits(hit2);
  proto1.addToWeights(1.0);
  proto1.addToHits(hit3);
  proto1.addToWeights(1.0);
  proto2.addToHits(hit4);
  proto2.addToWeights(1.0);
  proto2.addToHits(hit5);
  proto2.addToWeights(1.0);
  proto3.addToHits(hit6);
  proto3.addToWeights(1.0);

  auto clust_coll = std::make_unique<edm4eic::ClusterCollection>();
  auto clust1     = clust_coll->create();
  auto clust2     = clust_coll->create();
  auto clust3     = clust_coll->create();
  clust1.addToHits(hit1);
  clust1.addToHits(hit2);
  clust1.addToHits(hit3);
  clust2.addToHits(hit4);
  clust2.addToHits(hit5);
  clust3.addToHits(hit6);

  auto track_coll = std::make_unique<edm4eic::TrackCollection>();
  auto track1     = track_coll->create(0, edm4hep::Vector3f(0.0, 0.0, 0.0), edm4hep::Vector3f(-1.0, -1.0, -2.5));
  auto track2     = track_coll->create(0, edm4hep::Vector3f(0.0, 0.0, 0.0), edm4hep::Vector3f(-0.5, -0.5, -2.0));
  auto track3     = track_coll->create(0, edm4hep::Vector3f(0.0, 0.0, 0.0), edm4hep::Vector3f(0.0, -0.5, -2.0));
  auto track4     = track_coll->create(0, edm4hep::Vector3f(0.0, 0.0, 0.0), edm4hep::Vector3f(0.0, 0.0, -3.0));

  // link tracks and proto/clusters
  //   - proto/clust 1 <--- {track 1, track 2}
  //   - proto/clust 2 <--- {track 3}
  //   - proto/clust 3 <--- {track 4}
  auto proto_match_coll = std::make_unique<edm4eic::TrackProtoClusterMatchCollection>();
  auto protomatch1      = proto_match_coll->create();
  auto protomatch2      = proto_match_coll->create();
  auto protomatch3      = proto_match_coll->create();
  auto protomatch4      = proto_match_coll->create();
  protomatch1.setFrom(track1);
  protomatch1.setTo(proto1);
  protomatch2.setFrom(track2);
  protomatch2.setTo(proto1);
  protomatch3.setFrom(track3);
  protomatch3.setTo(proto2);
  protomatch4.setFrom(track4);
  protomatch4.setTo(proto3);

  auto clust_match_coll = std::make_unique<edm4eic::TrackClusterMatchCollection>();
  auto clustmatch1      = clust_match_coll->create();
  auto clustmatch2      = clust_match_coll->create();
  auto clustmatch3      = clust_match_coll->create();
  auto clustmatch4      = clust_match_coll->create();
  clustmatch1.setTrack(track1);
  clustmatch1.setCluster(clust1);
  clustmatch2.setTrack(track2);
  clustmatch2.setCluster(clust1);
  clustmatch3.setTrack(track3);
  clustmatch3.setCluster(clust2);
  clustmatch4.setTrack(track4);
  clustmatch4.setCluster(clust3);

  // configure reco algorithm to match EEEMCAL
  eicrecon::CalorimeterClusterRecoCoGConfig cfg_reco;
  cfg_reco.energyWeight    = "log";
  cfg_reco.sampFrac        = 1.0;
  cfg_reco.logWeightBase   = 3.6;
  cfg_reco.enableEtaBounds = false;

  eicrecon::CalorimeterClusterRecoCoG algo_reco("calorimeterClusterRecoCoG");
  algo_reco.level(algorithms::LogLevel::kDebug);
  algo_reco.applyConfig(cfg_reco);
  algo_reco.init();

  auto reco_coll      = std::make_unique<edm4eic::ClusterCollection>();
  auto hit_assoc_coll = std::make_unique<edm4eic::MCRecoCalorimeterHitAssociationCollection>();
  auto par_assoc_coll = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto hit_link_coll  = std::make_unique<edm4eic::MCRecoCalorimeterHitLinkCollection>();
  auto par_link_coll  = std::make_unique<edm4eic::MCRecoClusterParticleLinkCollection>();
  auto input_reco     = std::make_tuple(proto_coll.get(), hit_link_coll.get(), hit_assoc_coll.get());
  auto output_reco    = std::make_tuple(reco_coll.get(), par_link_coll.get(), par_assoc_coll.get());
#else
  auto input_reco     = std::make_tuple(proto_coll.get(), hit_assoc_coll.get());
  auto output_reco    = std::make_tuple(reco_coll.get(), par_assoc_coll.get());
#endif
  algo_reco.process(input_reco, output_reco);

  // output for next two tests
  auto reco_match_coll = std::make_unique<edm4eic::TrackClusterMatchCollection>();

  SECTION("algorithm produces correct number of outputs") {
    algo_promote.process({proto_match_coll.get(), proto_coll.get(), clust_coll.get()}, {reco_match_coll.get()});
    REQUIRE(reco_match_coll->size() == clust_match_coll->size());
  }

  SECTION("algorithm correctly matches clusters to tracks") {
    for (std::size_t idx = 0; idx < reco_match_coll->size(); ++idx) {
      REQUIRE((*reco_match_coll)[idx].getCluster() == (*clust_match_coll)[idx].getCluster());
      REQUIRE((*reco_match_coll)[idx].getTrack() == (*clust_match_coll)[idx].getTrack());
    }
  }

  // TODO
  //  - run algo on protoclusters + links
  //  - confirm that algo output matches manual case

}
