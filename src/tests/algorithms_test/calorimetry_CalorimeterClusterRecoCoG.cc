// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/unit_system.h>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"

using eicrecon::CalorimeterClusterRecoCoG;
using eicrecon::CalorimeterClusterRecoCoGConfig;

TEST_CASE( "the cluster assembly algorithm runs", "[CalorimeterClusterCoG]" ) {
  CalorimeterClusterRecoCoG algo("CalorimeterClusterRecoCoG");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterClusterRecoCoG");
  logger->set_level(spdlog::level::trace);

  CalorimeterClusterRecoCoGConfig cfg;

  cfg.energyWeight = "none";
  cfg.sampFrac = 1.;
  cfg.logWeightBase = 3.6;
  cfg.logWeightBaseCoeffs = {};
  cfg.logWeightBase_Eref = 50 * dd4hep::MeV;
  cfg.enableEtaBounds = false;

  algo.applyConfig(cfg);
  algo.init(logger);

  SECTION( "on a single cell" ) {
    // Inputs
    edm4eic::CalorimeterHitCollection hits_coll;
    edm4eic::ProtoClusterCollection proto_coll;
    hits_coll.create(
      0xBEEF, // std::uint64_t cellID,
      5.0 * edm4eic::unit::MeV, // float energy,
      0.0 * edm4eic::unit::MeV, // float energyError,
      0.0 * edm4eic::unit::ns, // float time,
      0.0 * edm4eic::unit::ns, // float timeError,
      edm4hep::Vector3f(1.0 * edm4eic::unit::mm, 2.0 * edm4eic::unit::mm, 3.0 * edm4eic::unit::mm), // edm4hep::Vector3f position,
      edm4hep::Vector3f(0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm), // edm4hep::Vector3f dimension,
      0, // std::int32_t sector,
      0, // std::int32_t layer,
      edm4hep::Vector3f(0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm) // edm4hep::Vector3f local
    );
    auto proto = proto_coll.create();
    proto.addToHits(hits_coll[0]);
    proto.addToWeights(123.0);

    edm4hep::SimCalorimeterHitCollection mchit_coll;
    bool with_matching_mc_hit;
    edm4hep::MCParticleCollection part_coll;
    edm4hep::CaloHitContributionCollection contrib_coll;
    SECTION( "with matching MC hit" ) {
      with_matching_mc_hit = true;
      part_coll.create();
      auto mchit = mchit_coll.create(
        0xBEEF, // std::uint64_t cellID,
        6.0 * edm4eic::unit::MeV, // float energy,
        edm4hep::Vector3f(0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm) // edm4hep::Vector3f position
      );
      contrib_coll.create(
        0, // std::int32_t PDG,
        0 * edm4eic::unit::MeV, // float energy,
        0 * edm4eic::unit::ns, // float time,
        edm4hep::Vector3f(0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm, 0.0 * edm4eic::unit::mm) // edm4hep::Vector3f stepPosition
      );
      contrib_coll[0].setParticle(part_coll[0]);
      mchit.addToContributions(contrib_coll[0]);
    }

    SECTION( "without matching MC hit" ) {
      with_matching_mc_hit = false;
    }

    // Outputs
    auto clust_coll = std::make_unique<edm4eic::ClusterCollection>();
    auto assoc_coll = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

    algo.process({&proto_coll, &mchit_coll}, {clust_coll.get(), assoc_coll.get()});

    REQUIRE( (*clust_coll).size() == 1 );
    REQUIRE( (*clust_coll)[0].hits_size() == 1 );
    REQUIRE( (*clust_coll)[0].getHits(0).getEnergy() == hits_coll[0].getEnergy() );
    REQUIRE( (*clust_coll)[0].getHits(0).getEnergyError() == hits_coll[0].getEnergyError() );
    REQUIRE( (*clust_coll)[0].getHits(0).getTime() == hits_coll[0].getTime() );
    REQUIRE( (*clust_coll)[0].getHits(0).getTimeError() == hits_coll[0].getTimeError() );
    REQUIRE( (*clust_coll)[0].getHits(0).getPosition() == hits_coll[0].getPosition() );

    if (with_matching_mc_hit) {
      REQUIRE( (*assoc_coll).size() == 1 );
      REQUIRE( (*assoc_coll)[0].getSim() == part_coll[0] );
      REQUIRE( (*assoc_coll)[0].getSim().id() == part_coll[0].id() );
      REQUIRE( (*assoc_coll)[0].getRec() == (*clust_coll)[0] );
      REQUIRE( (*assoc_coll)[0].getRec().id() == (*clust_coll)[0].id() );
    } else {
      REQUIRE( (*assoc_coll).size() == 0 );
    }
  }
}
