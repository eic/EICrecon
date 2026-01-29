// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Sebouh Paul, Dmitry Kalinkin

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <cmath>
#include <memory>
#include <string>
#include <tuple>

#include "algorithms/calorimetry/CalorimeterClusterShape.h"
#include "algorithms/calorimetry/CalorimeterClusterShapeConfig.h"

using eicrecon::CalorimeterClusterShape;
using eicrecon::CalorimeterClusterShapeConfig;

TEST_CASE("the calorimeter CoG algorithm runs", "[CalorimeterClusterShape]") {
  const float EPSILON = 1e-5;

  CalorimeterClusterShape algo("CalorimeterClusterShape");

  std::shared_ptr<spdlog::logger> logger =
      spdlog::default_logger()->clone("CalorimeterClusterShape");
  logger->set_level(spdlog::level::trace);

  CalorimeterClusterShapeConfig cfg;
  cfg.longitudinalShowerInfoAvailable = true;

  algo.applyConfig(cfg);
  algo.init();

  edm4eic::CalorimeterHitCollection hits_coll;
  edm4eic::MCRecoClusterParticleAssociationCollection assoc_in_coll;
  edm4eic::ClusterCollection clust_in_coll;
  auto assoc_out_coll = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
  auto clust_out_coll = std::make_unique<edm4eic::ClusterCollection>();

  auto hit1 = hits_coll.create();
  hit1.setCellID(0);
  hit1.setEnergy(0.1 * edm4eic::unit::GeV);
  hit1.setEnergyError(0);
  hit1.setTime(0);
  hit1.setTimeError(0);
  hit1.setPosition(edm4hep::Vector3f{0, 0, 1 * edm4eic::unit::mm});
  hit1.setDimension({0, 0, 0});
  hit1.setLocal(edm4hep::Vector3f{0, 0, 1 * edm4eic::unit::mm});

  auto hit2 = hits_coll.create();
  hit2.setCellID(1);
  hit2.setEnergy(0.1 * edm4eic::unit::GeV);
  hit2.setEnergyError(0);
  hit2.setTime(0);
  hit2.setTimeError(0);
  hit2.setPosition(edm4hep::Vector3f{-1 * edm4eic::unit::mm, 0, 2 * edm4eic::unit::mm});
  hit2.setDimension({0, 0, 0});
  hit2.setLocal(edm4hep::Vector3f{-1 * edm4eic::unit::mm, 0, 2 * edm4eic::unit::mm});

  // Create a cluster with 2 hits
  auto clust_in = clust_in_coll.create();
  clust_in.addToHits(hit1);
  clust_in.addToHitContributions(hit1.getEnergy());
  clust_in.addToHits(hit2);
  clust_in.addToHitContributions(hit2.getEnergy());
  clust_in.setNhits(clust_in.hits_size());
  clust_in.setEnergy(hit1.getEnergy() + hit2.getEnergy());
  clust_in.setPosition((hit1.getPosition() + hit2.getPosition()) / 2);

  auto assoc_in = assoc_in_coll.create();
  assoc_in.setWeight(0.123);
  assoc_in.setRec(clust_in);
  // assoc_in.setSim(...);

  // Constructing input and output as per the algorithm's expected signature
  auto input = std::make_tuple(&clust_in_coll, &assoc_in_coll);
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoClusterParticleLinkCollection link_out_coll;
  auto output = std::make_tuple(clust_out_coll.get(), &link_out_coll, assoc_out_coll.get());
#else
  auto output = std::make_tuple(clust_out_coll.get(), assoc_out_coll.get());
#endif

  algo.process(input, output);

  REQUIRE(clust_out_coll->size() == 1);
  auto clust_out = (*clust_out_coll)[0];
  REQUIRE(clust_in.getNhits() == clust_out.getNhits());

  REQUIRE_THAT(clust_out.getIntrinsicTheta(), Catch::Matchers::WithinAbs(M_PI / 4, EPSILON));
  // std::abs() checks if we land on -M_PI
  REQUIRE_THAT(std::abs(clust_out.getIntrinsicPhi()), Catch::Matchers::WithinAbs(M_PI, EPSILON));

  REQUIRE(assoc_out_coll->size() == 1);
  REQUIRE((*assoc_out_coll)[0].getRec() == clust_out);
  REQUIRE((*assoc_out_coll)[0].getWeight() == assoc_in.getWeight());
}
