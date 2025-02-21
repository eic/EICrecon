// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul


#include <catch2/catch_test_macros.hpp>            // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>      // for CalorimeterHitCollection, MutableCalorimeterHit, CalorimeterHitMutableCollectionIterator
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/Vector2i.h>
#include <edm4hep/Vector3d.h>
#if EDM4EIC_VERSION_MAJOR >= 7
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#endif
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <spdlog/common.h>                         // for level_enum
#include <spdlog/logger.h>                         // for logger
#include <spdlog/spdlog.h>                         // for default_logger
#include <memory>                                  // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <tuple>
#include <vector>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"        // for CalorimeterClusterRecoCoG
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"  // for CalorimeterClusterRecoCoGConfig
#include "algorithms/calorimetry/CalorimeterClusterShape.h"          // for CalorimeterClusterShape
#include "algorithms/calorimetry/CalorimeterClusterShapeConfig.h"    // for CalorimeterClusterShapeConfig

using eicrecon::CalorimeterClusterRecoCoG;
using eicrecon::CalorimeterClusterRecoCoGConfig;
using eicrecon::CalorimeterClusterShape;
using eicrecon::CalorimeterClusterShapeConfig;

using edm4eic::CalorimeterHit;

TEST_CASE( "the calorimeter CoG and shape algorithm runs", "[CalorimeterClusterRecoCoG]" ) {
  const float EPSILON = 1e-5;

  CalorimeterClusterRecoCoG algo_reco("CalorimeterClusterRecoCoG");
  CalorimeterClusterShape algo_shape("CalorimeterClusterShape");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterClusterRecoCoG");
  logger->set_level(spdlog::level::trace);

  // configure RecoCoG
  CalorimeterClusterRecoCoGConfig cfg_reco;
  cfg_reco.energyWeight = "log";
  cfg_reco.sampFrac = 0.0203;
  cfg_reco.logWeightBaseCoeffs = {5.0,0.65,0.31};
  cfg_reco.logWeightBase_Eref = 50 * edm4eic::unit::GeV;

  // configure Shape
  CalorimeterClusterShapeConfig cfg_shape;
  cfg_shape.longitudinalShowerInfoAvailable = true;

  // apply configurations
  algo_reco.applyConfig(cfg_reco);
  algo_reco.init();

  algo_shape.applyConfig(cfg_shape);
  algo_shape.init();

  edm4hep::RawCalorimeterHitCollection rawhits_coll;
  edm4eic::CalorimeterHitCollection hits_coll;
  edm4eic::ProtoClusterCollection pclust_coll;
  edm4hep::SimCalorimeterHitCollection simhits_coll;
#if EDM4EIC_VERSION_MAJOR >= 7
  edm4eic::MCRecoCalorimeterHitAssociationCollection hitassocs_coll;
#endif
  edm4hep::CaloHitContributionCollection contribs_coll;
  edm4hep::MCParticleCollection mcparts_coll;
  auto assoc_coll_reco = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
  auto clust_coll_reco = std::make_unique<edm4eic::ClusterCollection>();

  //create a protocluster with 3 hits
  auto pclust = pclust_coll.create();
  edm4hep::Vector3f position({0, 0, 1 * edm4eic::unit::mm});

  auto rawhit1 = rawhits_coll.create();

  auto hit1 = hits_coll.create();
  hit1.setCellID(0);
  hit1.setEnergy(0.1 * edm4eic::unit::GeV);
  hit1.setEnergyError(0);
  hit1.setTime(0);
  hit1.setTimeError(0);
  hit1.setPosition(position);
  hit1.setDimension({0,0,0});
  hit1.setLocal(position);
#if EDM4EIC_VERSION_MAJOR >= 7
  hit1.setRawHit(rawhit1);
#endif
  pclust.addToHits(hit1);
  pclust.addToWeights(1);

  auto mcpart11 = mcparts_coll.create(
    11, // std::int32_t PDG
    1, // std::int32_t generatorStatus
    0, // std::int32_t simulatorStatus
    -1., // float charge
    0., // float time
    0., // double mass
    edm4hep::Vector3d(), // edm4hep::Vector3d vertex
    edm4hep::Vector3d(), // edm4hep::Vector3d endpoint
    edm4hep::Vector3f(), // edm4hep::Vector3f momentum
    edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
    edm4hep::Vector3f(), // edm4hep::Vector3f spin
    edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
  );

  auto mcpart12 = mcparts_coll.create(
    22, // std::int32_t PDG
    0, // std::int32_t generatorStatus
    (0x1 << edm4hep::MCParticle::BITCreatedInSimulation), // std::int32_t simulatorStatus
    0., // float charge
    0., // float time
    0., // double mass
    edm4hep::Vector3d(), // edm4hep::Vector3d vertex
    edm4hep::Vector3d(), // edm4hep::Vector3d endpoint
    edm4hep::Vector3f(), // edm4hep::Vector3f momentum
    edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
    edm4hep::Vector3f(), // edm4hep::Vector3f spin
    edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
  );

  mcpart12.addToParents(mcpart11);
  mcpart11.addToDaughters(mcpart12);

  auto contrib11 = contribs_coll.create(
    0, // int32_t PDG
    0.05 * edm4eic::unit::GeV, // float energy
    0.0, // float time
    edm4hep::Vector3f() // edm4hep::Vector3f stepPosition
  );
  contrib11.setParticle(mcpart11);
  auto contrib12 = contribs_coll.create(
    0, // int32_t PDG
    0.05 * edm4eic::unit::GeV, // float energy
    0.0, // float time
    edm4hep::Vector3f() // edm4hep::Vector3f stepPosition
  );
  contrib12.setParticle(mcpart12);

  auto simhit1 = simhits_coll.create();
  simhit1.setCellID(hit1.getCellID());
  simhit1.setEnergy(0.1 * edm4eic::unit::GeV);
  simhit1.setPosition(hit1.getPosition());
  simhit1.addToContributions(contrib11);
  simhit1.addToContributions(contrib12);

#if EDM4EIC_VERSION_MAJOR >= 7
  auto hitassoc1 = hitassocs_coll->create();
  hitassoc1.setRawHit(rawhit1);
  hitassoc1.setSimHit(simhit1);
#endif

  auto rawhit2 = rawhits_coll.create();

  position={-1 * edm4eic::unit::mm, 0, 2 * edm4eic::unit::mm};
  auto hit2 = hits_coll.create();
  hit2.setCellID(1);
  hit2.setEnergy(0.1 * edm4eic::unit::GeV);
  hit2.setEnergyError(0);
  hit2.setTime(0);
  hit2.setTimeError(0);
  hit2.setPosition(position);
  hit2.setDimension({0,0,0});
  hit2.setLocal(position);
#if EDM4EIC_VERSION_MAJOR >= 7
  hit2.setRawHit(rawhit2);
#endif
  pclust.addToHits(hit2);
  pclust.addToWeights(1);

  auto mcpart2 = mcparts_coll.create(
    211, // std::int32_t PDG
    0, // std::int32_t generatorStatus
    (0x1 << edm4hep::MCParticle::BITCreatedInSimulation), // std::int32_t simulatorStatus
    0., // float charge
    0., // float time
    0., // double mass
    edm4hep::Vector3d(), // edm4hep::Vector3d vertex
    edm4hep::Vector3d(), // edm4hep::Vector3d endpoint
    edm4hep::Vector3f(), // edm4hep::Vector3f momentum
    edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
    edm4hep::Vector3f(), // edm4hep::Vector3f spin
    edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
  );

  auto contrib2 = contribs_coll.create(
    0, // int32_t PDG
    0.1 * edm4eic::unit::GeV, // float energy
    0.0, // float time
    edm4hep::Vector3f() // edm4hep::Vector3f stepPosition
  );
  contrib2.setParticle(mcpart2);

  auto simhit2 = simhits_coll.create();
  simhit2.setCellID(hit2.getCellID());
  simhit2.setEnergy(0.1 * edm4eic::unit::GeV);
  simhit2.setPosition(hit2.getPosition());
  simhit2.addToContributions(contrib2);

#if EDM4EIC_VERSION_MAJOR >= 7
  auto hitassoc2 = hitassocs_coll->create();
  hitassoc2.setRawHit(rawhit2);
  hitassoc2.setSimHit(simhit2);
#endif

 // Constructing input and output as per the algorithm's expected signature
#if EDM4EIC_VERSION_MAJOR >= 7
  auto input_reco = std::make_tuple(&pclust_coll, &hitassocs_coll);
#else
  auto input_reco = std::make_tuple(&pclust_coll, &simhits_coll);
#endif
  auto output_reco = std::make_tuple(clust_coll_reco.get(), assoc_coll_reco.get());

  algo_reco.process(input_reco, output_reco);

  // create collections to hold shape output
  auto assoc_coll_shape = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
  auto clust_coll_shape = std::make_unique<edm4eic::ClusterCollection>();

  // collect input/output for shape algorithm
  auto input_shape = std::make_tuple(clust_coll_reco.get(), assoc_coll_reco.get());
  auto output_shape = std::make_tuple(clust_coll_shape.get(), assoc_coll_shape.get());

  // Run shape calculation on output of RecoCoG
  algo_shape.process(input_shape, output_shape);

  REQUIRE(clust_coll_shape->size() == 1);
  auto clust = (*clust_coll_shape)[0];
  REQUIRE_THAT(clust.getIntrinsicTheta(), Catch::Matchers::WithinAbs(M_PI / 4, EPSILON));
  // std::abs() checks if we land on -M_PI
  REQUIRE_THAT(std::abs(clust.getIntrinsicPhi()), Catch::Matchers::WithinAbs(M_PI, EPSILON));

  REQUIRE(assoc_coll_shape->size() == 2);

  // Half of the energy comes from mcpart11 and its daughter mcpart12
  REQUIRE_THAT((*assoc_coll_shape)[0].getWeight(), Catch::Matchers::WithinAbs(0.5, EPSILON));
  REQUIRE((*assoc_coll_shape)[0].getRec() == clust);
  REQUIRE((*assoc_coll_shape)[0].getSim() == mcpart11);

  // Half of the energy comes from mcpart2
  REQUIRE_THAT((*assoc_coll_shape)[1].getWeight(), Catch::Matchers::WithinAbs(0.5, EPSILON));
  REQUIRE((*assoc_coll_shape)[1].getRec() == clust);
  REQUIRE((*assoc_coll_shape)[1].getSim() == mcpart2);

}
