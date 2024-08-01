// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul


#include <Evaluator/DD4hepUnits.h>                 // for MeV, mm, keV, ns
#include <catch2/catch_test_macros.hpp>            // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>      // for CalorimeterHitCollection, MutableCalorimeterHit, CalorimeterHitMutableCollectionIterator
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>                      // for Vector3f
#include <math.h>
#include <spdlog/common.h>                         // for level_enum
#include <spdlog/logger.h>                         // for logger
#include <spdlog/spdlog.h>                         // for default_logger
#include <memory>                                  // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <tuple>
#include <vector>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"        // for CalorimeterClusterRecoCoG
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"  // for CalorimeterClusterRecoCoGConfig

using eicrecon::CalorimeterClusterRecoCoG;
using eicrecon::CalorimeterClusterRecoCoGConfig;

using edm4eic::CalorimeterHit;

TEST_CASE( "the calorimeter CoG algorithm runs", "[CalorimeterClusterRecoCoG]" ) {
  const float EPSILON = 1e-5;

  CalorimeterClusterRecoCoG algo("CalorimeterClusterRecoCoG");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterClusterRecoCoG");
  logger->set_level(spdlog::level::trace);

  CalorimeterClusterRecoCoGConfig cfg;
  cfg.energyWeight = "log";
  cfg.sampFrac = 0.0203;
  cfg.logWeightBaseCoeffs = {5.0,0.65,0.31};
  cfg.logWeightBase_Eref = 50*dd4hep::GeV;
  cfg.longitudinalShowerInfoAvailable = true;


  algo.applyConfig(cfg);
  algo.init();

  edm4eic::CalorimeterHitCollection hits_coll;
  edm4eic::ProtoClusterCollection pclust_coll;
  edm4hep::SimCalorimeterHitCollection simhits;
  auto assoc = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
  auto clust_coll = std::make_unique<edm4eic::ClusterCollection>();

  //create a protocluster with 3 hits
  auto pclust = pclust_coll.create();
  edm4hep::Vector3f position({0, 0, 1 *dd4hep::mm});

  auto hit1 = hits_coll.create();
  hit1.setCellID(0);
  hit1.setEnergy(0.1*dd4hep::GeV);
  hit1.setEnergyError(0);
  hit1.setTime(0);
  hit1.setTimeError(0);
  hit1.setPosition(position);
  hit1.setDimension({0,0,0});
  hit1.setLocal(position);
  pclust.addToHits(hit1);
  pclust.addToWeights(1);

  position={-1 * dd4hep::mm, 0, 2 * dd4hep::mm};
  auto hit2 = hits_coll.create();
  hit2.setCellID(0);
  hit2.setEnergy(0.1*dd4hep::GeV);
  hit2.setEnergyError(0);
  hit2.setTime(0);
  hit2.setTimeError(0);
  hit2.setPosition(position);
  hit2.setDimension({0,0,0});
  hit2.setLocal(position);
  pclust.addToHits(hit2);
  pclust.addToWeights(1);

  // Constructing input and output as per the algorithm's expected signature
  auto input = std::make_tuple(&pclust_coll, &simhits);
  auto output = std::make_tuple(clust_coll.get(), assoc.get());

  algo.process(input, output);


  for (auto clust : *clust_coll){
    REQUIRE_THAT(clust.getIntrinsicTheta(), Catch::Matchers::WithinAbs(M_PI / 4, EPSILON));
    // std::abs() checks if we land on -M_PI
    REQUIRE_THAT(std::abs(clust.getIntrinsicPhi()), Catch::Matchers::WithinAbs(M_PI, EPSILON));
  }


}
