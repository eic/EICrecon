// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul

#include <DD4hep/Detector.h>                       // for Detector
#include <DD4hep/IDDescriptor.h>                   // for IDDescriptor
#include <DD4hep/Readout.h>                        // for Readout
#include <Evaluator/DD4hepUnits.h>                 // for MeV, mm, keV, ns
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>            // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <edm4eic/CalorimeterHitCollection.h>      // for CalorimeterHitCollection, MutableCalorimeterHit, CalorimeterHitMutableCollectionIterator
#include <edm4hep/Vector3f.h>                      // for Vector3f
#include <spdlog/common.h>                         // for level_enum
#include <spdlog/logger.h>                         // for logger
#include <spdlog/spdlog.h>                         // for default_logger
#include <stddef.h>                                // for size_t
#include <array>                                   // for array
#include <cmath>                                   // for sqrt, abs
#include <gsl/pointers>
#include <memory>                                  // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <utility>                                 // for pair

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"        // for CalorimeterClusterRecoCoG
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"  // for CalorimeterClusterRecoCoGConfig

using eicrecon::CalorimeterClusterRecoCoG;
using eicrecon::CalorimeterClusterRecoCoGConfig;

using edm4eic::CalorimeterHit;

TEST_CASE( "the calorimeter CoG algorithm runs", "[CalorimeterClusterRecoCoG]" ) {
  CalorimeterClusterRecoCoG algo("CalorimeterClusterRecoCoG");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterClusterRecoCoG");
  logger->set_level(spdlog::level::trace);

  CalorimeterClusterRecoCoGConfig cfg;
  cfg.energyWeight = "log";
  cfg.sampFrac = 0.0203,
  cfg.logWeightBaseCoeffs={5.0,0.65,0.31},
  cfg.logWeightBase_Eref=50*dd4hep::GeV,


  algo.applyConfig(cfg);
  algo.init();

  edm4eic::CalorimeterHitCollection hits_coll;
  edm4eic::ProtoClusterCollection pclust_coll;
  edm4hep::SimCalorimeterHitCollection simhits;
  auto assoc = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
  auto clust_coll = std::make_unique<edm4eic::ClusterCollection>();

  //create a protocluster with 3 hits
  auto pclust = pclust_coll.create();
  edm4hep::Vector3f position({0,0,0*dd4hep::mm});

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

  position={0,0, 1*dd4hep::mm};
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
  
  position={0,0, 2*dd4hep::mm};
  auto hit3 = hits_coll.create();//0, 0.1*dd4hep::GeV, 0,0,0,position, {0,0,0}, 0,0, position);
  hit3.setCellID(0);
  hit3.setEnergy(0.1*dd4hep::GeV);
  hit3.setEnergyError(0);
  hit3.setTime(0);
  hit3.setTimeError(0);
  hit3.setPosition(position);
  hit3.setDimension({0,0,0});
  hit3.setLocal(position);
  pclust.addToHits(hit3);
  pclust.addToWeights(1);pclust.addToWeights(1);pclust.addToWeights(1);

  // Constructing input and output as per the algorithm's expected signature
  auto input = std::make_tuple(&pclust_coll, &simhits);
  auto output = std::make_tuple(clust_coll.get(), assoc.get());

  algo.process(input, output);


  for (auto clust : *clust_coll){
    // require that this cluster's axis is 0,0,1
    REQUIRE(clust.getShapeParameters()[7] == 0);
    REQUIRE(clust.getShapeParameters()[8] == 0);
    REQUIRE(abs(clust.getShapeParameters()[9]) == 1);
  }


}
