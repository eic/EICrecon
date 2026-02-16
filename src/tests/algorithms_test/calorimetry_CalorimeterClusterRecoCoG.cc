// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#endif
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/EDM4hepVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
#include <edm4hep/Vector2i.h>
#endif
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <deque>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"

using eicrecon::CalorimeterClusterRecoCoG;
using eicrecon::CalorimeterClusterRecoCoGConfig;

TEST_CASE("the calorimeter CoG algorithm runs", "[CalorimeterClusterRecoCoG]") {
  const float EPSILON = 1e-5;

  CalorimeterClusterRecoCoG algo("CalorimeterClusterRecoCoG");

  std::shared_ptr<spdlog::logger> logger =
      spdlog::default_logger()->clone("CalorimeterClusterRecoCoG");
  logger->set_level(spdlog::level::trace);

  CalorimeterClusterRecoCoGConfig cfg;
  cfg.energyWeight        = "log";
  cfg.sampFrac            = 0.0203;
  cfg.logWeightBaseCoeffs = {5.0, 0.65, 0.31};
  cfg.logWeightBase_Eref  = 50 * edm4eic::unit::GeV;

  algo.applyConfig(cfg);
  algo.init();

  edm4hep::RawCalorimeterHitCollection rawhits_coll;
  edm4eic::CalorimeterHitCollection hits_coll;
  edm4eic::ProtoClusterCollection pclust_coll;
  edm4hep::SimCalorimeterHitCollection simhits_coll;
  edm4eic::MCRecoCalorimeterHitAssociationCollection hitassocs_coll;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoCalorimeterHitLinkCollection hitlinks_coll;
#endif
  edm4hep::CaloHitContributionCollection contribs_coll;
  edm4hep::MCParticleCollection mcparts_coll;
  auto assoc_coll = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto link_coll = std::make_unique<edm4eic::MCRecoClusterParticleLinkCollection>();
#endif
  auto clust_coll = std::make_unique<edm4eic::ClusterCollection>();

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
  hit1.setDimension({0, 0, 0});
  hit1.setLocal(position);
  hit1.setRawHit(rawhit1);
  pclust.addToHits(hit1);
  pclust.addToWeights(1);

  auto mcpart11 = mcparts_coll.create(11,                  // std::int32_t PDG
                                      1,                   // std::int32_t generatorStatus
                                      0,                   // std::int32_t simulatorStatus
                                      -1.,                 // float charge
                                      0.,                  // float time
                                      0.,                  // double mass
                                      edm4hep::Vector3d(), // edm4hep::Vector3d vertex
                                      edm4hep::Vector3d(), // edm4hep::Vector3d endpoint
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 1)
                                      edm4hep::Vector3f(), // edm4hep::Vector3f momentum
                                      edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
#else
                                      edm4hep::Vector3d(), // edm4hep::Vector3d momentum
                                      edm4hep::Vector3d(), // edm4hep::Vector3d momentumAtEndpoint
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 3)
                                      edm4hep::Vector3f() // edm4hep::Vector3f spin
#else
                                      9 // int32_t helicity (9 if unset)
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
                                      ,
                                      edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
#endif
  );

  auto mcpart12 = mcparts_coll.create(
      22,                                                   // std::int32_t PDG
      0,                                                    // std::int32_t generatorStatus
      (0x1 << edm4hep::MCParticle::BITCreatedInSimulation), // std::int32_t simulatorStatus
      0.,                                                   // float charge
      0.,                                                   // float time
      0.,                                                   // double mass
      edm4hep::Vector3d(),                                  // edm4hep::Vector3d vertex
      edm4hep::Vector3d(),                                  // edm4hep::Vector3d endpoint
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 1)
      edm4hep::Vector3f(), // edm4hep::Vector3f momentum
      edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
#else
      edm4hep::Vector3d(), // edm4hep::Vector3d momentum
      edm4hep::Vector3d(), // edm4hep::Vector3d momentumAtEndpoint
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 3)
      edm4hep::Vector3f() // edm4hep::Vector3f spin
#else
      9 // int32_t helicity (9 if unset)
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
      ,
      edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
#endif
  );

  mcpart12.addToParents(mcpart11);
  mcpart11.addToDaughters(mcpart12);

  auto contrib11 = contribs_coll.create(0,                         // int32_t PDG
                                        0.05 * edm4eic::unit::GeV, // float energy
                                        0.0,                       // float time
                                        edm4hep::Vector3f()        // edm4hep::Vector3f stepPosition
  );
  contrib11.setParticle(mcpart11);
  auto contrib12 = contribs_coll.create(0,                         // int32_t PDG
                                        0.05 * edm4eic::unit::GeV, // float energy
                                        0.0,                       // float time
                                        edm4hep::Vector3f()        // edm4hep::Vector3f stepPosition
  );
  contrib12.setParticle(mcpart12);

  auto simhit1 = simhits_coll.create();
  simhit1.setCellID(hit1.getCellID());
  simhit1.setEnergy(0.1 * edm4eic::unit::GeV);
  simhit1.setPosition(hit1.getPosition());
  simhit1.addToContributions(contrib11);
  simhit1.addToContributions(contrib12);

  auto hitassoc1 = hitassocs_coll.create();
  hitassoc1.setRawHit(rawhit1);
  hitassoc1.setSimHit(simhit1);
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto hitlink1 = hitlinks_coll.create();
  hitlink1.setFrom(rawhit1);
  hitlink1.setTo(simhit1);
#endif

  auto rawhit2 = rawhits_coll.create();

  position  = {-1 * edm4eic::unit::mm, 0, 2 * edm4eic::unit::mm};
  auto hit2 = hits_coll.create();
  hit2.setCellID(1);
  hit2.setEnergy(0.1 * edm4eic::unit::GeV);
  hit2.setEnergyError(0);
  hit2.setTime(0);
  hit2.setTimeError(0);
  hit2.setPosition(position);
  hit2.setDimension({0, 0, 0});
  hit2.setLocal(position);
  hit2.setRawHit(rawhit2);
  pclust.addToHits(hit2);
  pclust.addToWeights(1);

  auto mcpart2 = mcparts_coll.create(
      211,                                                  // std::int32_t PDG
      0,                                                    // std::int32_t generatorStatus
      (0x1 << edm4hep::MCParticle::BITCreatedInSimulation), // std::int32_t simulatorStatus
      0.,                                                   // float charge
      0.,                                                   // float time
      0.,                                                   // double mass
      edm4hep::Vector3d(),                                  // edm4hep::Vector3d vertex
      edm4hep::Vector3d(),                                  // edm4hep::Vector3d endpoint
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 1)
      edm4hep::Vector3f(), // edm4hep::Vector3f momentum
      edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
#else
      edm4hep::Vector3d(), // edm4hep::Vector3d momentum
      edm4hep::Vector3d(), // edm4hep::Vector3d momentumAtEndpoint
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 3)
      edm4hep::Vector3f() // edm4hep::Vector3f spin
#else
      9 // int32_t helicity (9 if unset)
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
      ,
      edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
#endif
  );

  auto contrib2 = contribs_coll.create(0,                        // int32_t PDG
                                       0.1 * edm4eic::unit::GeV, // float energy
                                       0.0,                      // float time
                                       edm4hep::Vector3f()       // edm4hep::Vector3f stepPosition
  );
  contrib2.setParticle(mcpart2);

  auto simhit2 = simhits_coll.create();
  simhit2.setCellID(hit2.getCellID());
  simhit2.setEnergy(0.1 * edm4eic::unit::GeV);
  simhit2.setPosition(hit2.getPosition());
  simhit2.addToContributions(contrib2);

  auto hitassoc2 = hitassocs_coll.create();
  hitassoc2.setRawHit(rawhit2);
  hitassoc2.setSimHit(simhit2);

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto hitlink2 = hitlinks_coll.create();
  hitlink2.setFrom(rawhit2);
  hitlink2.setTo(simhit2);
#endif

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto input  = std::make_tuple(&pclust_coll, &hitassocs_coll, &hitlinks_coll);
  auto output = std::make_tuple(clust_coll.get(), assoc_coll.get(), link_coll.get());
#else
  auto input  = std::make_tuple(&pclust_coll, &hitassocs_coll);
  auto output = std::make_tuple(clust_coll.get(), assoc_coll.get());
#endif

  algo.process(input, output);

  REQUIRE(clust_coll->size() == 1);
  auto clust = (*clust_coll)[0];

  REQUIRE(assoc_coll->size() == 2);
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  REQUIRE(link_coll->size() == 2);
#endif

  // Half of the energy comes from mcpart11 and its daughter mcpart12
  REQUIRE_THAT((*assoc_coll)[0].getWeight(), Catch::Matchers::WithinAbs(0.5, EPSILON));
  REQUIRE((*assoc_coll)[0].getRec() == clust);
  REQUIRE((*assoc_coll)[0].getSim() == mcpart11);

  // Half of the energy comes from mcpart2
  REQUIRE_THAT((*assoc_coll)[1].getWeight(), Catch::Matchers::WithinAbs(0.5, EPSILON));
  REQUIRE((*assoc_coll)[1].getRec() == clust);
  REQUIRE((*assoc_coll)[1].getSim() == mcpart2);

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  // Half of the energy comes from mcpart11 and its daughter mcpart12
  REQUIRE_THAT((*link_coll)[0].getWeight(), Catch::Matchers::WithinAbs(0.5, EPSILON));
  REQUIRE((*link_coll)[0].getFrom() == clust);
  REQUIRE((*link_coll)[0].getTo() == mcpart11);

  // Half of the energy comes from mcpart2
  REQUIRE_THAT((*link_coll)[1].getWeight(), Catch::Matchers::WithinAbs(0.5, EPSILON));
  REQUIRE((*link_coll)[1].getFrom() == clust);
  REQUIRE((*link_coll)[1].getTo() == mcpart2);
#endif
}
