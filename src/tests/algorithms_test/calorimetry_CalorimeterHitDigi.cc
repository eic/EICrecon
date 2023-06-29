// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <catch2/catch_test_macros.hpp>
#include <edm4hep/MutableSimCalorimeterHit.h>
#include <spdlog/logger.h>

#include <common/unit_system.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>


TEST_CASE( "the clustering algorithm runs", "[CalorimeterHitDigi]" ) {
  CalorimeterHitDigi algo;

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterHitDigi");
  logger->set_level(spdlog::level::trace);

  algo.m_threshold = 0. * unit::GeV;
  algo.m_corrMeanScale = 1.;

  // Keep smearing parameters at zero
  algo.m_pedSigmaADC = 0;
  algo.m_tRes = 0. * unit::ns;
  algo.u_eRes = {0. * sqrt(unit::GeV), 0., 0. * unit::GeV};

  SECTION( "single hit with couple contributions" ) {
    algo.m_capADC = 555;
    algo.m_dyRangeADC = 5.0 * unit::GeV;
    algo.m_pedMeanADC = 123;
    algo.m_resolutionTDC = 1.0 * unit::ns;
    algo.AlgorithmInit(logger);
    algo.AlgorithmChangeRun();

    auto mhit = edm4hep::MutableSimCalorimeterHit {
      0xABABABAB, // std::uint64_t cellID
      1.0 * unit::GeV, // float energy
      {0. /* mm */, 0. /* mm */, 0. /* mm */}, // edm4hep::Vector3f position
    };
    mhit.addToContributions({
      0, // std::int32_t PDG
      0.5 * unit::GeV, // float energy
      7.0 * unit::ns, // float time
      {0. /* mm */, 0. /* mm */, 0. /* mm */}, // edm4hep::Vector3f stepPosition
    });
    mhit.addToContributions({
      0, // std::int32_t PDG
      0.5 * unit::GeV, // float energy
      9.0 * unit::ns, // float time
      {0. /* mm */, 0. /* mm */, 0. /* mm */}, // edm4hep::Vector3f stepPosition
    });
    algo.simhits = {
      new edm4hep::SimCalorimeterHit(mhit),
    };

    algo.AlgorithmProcess();

    REQUIRE( algo.rawhits.size() == 1 );
    REQUIRE( algo.rawhits[0]->getCellID() == 0xABABABAB);
    REQUIRE( algo.rawhits[0]->getAmplitude() == 123 + 111 );
    REQUIRE( algo.rawhits[0]->getTimeStamp() == 7 ); // currently, earliest contribution is returned
  }
}
