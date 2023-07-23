// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <catch2/catch_test_macros.hpp>
#include <edm4hep/MutableSimCalorimeterHit.h>
#include <spdlog/logger.h>

#include <algorithms/calorimetry/CalorimeterHitDigi.h>


TEST_CASE( "the clustering algorithm runs", "[CalorimeterHitDigi]" ) {
  CalorimeterHitDigi algo;

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("CalorimeterHitDigi");
  logger->set_level(spdlog::level::trace);

  algo.m_threshold = 0. /* GeV */;
  algo.m_corrMeanScale = 1.;

  // Keep smearing parameters at zero
  algo.m_pedSigmaADC = 0;
  algo.m_tRes = 0. * dd4hep::ns;
  algo.u_eRes = {0. * sqrt(dd4hep::GeV), 0., 0. * dd4hep::GeV};

  SECTION( "single hit with couple contributions" ) {
    algo.m_capADC = 555;
    algo.m_dyRangeADC = 5.0 /* GeV */;
    algo.m_pedMeanADC = 123;
    algo.m_resolutionTDC = 1.0 * dd4hep::ns;
    algo.AlgorithmInit(logger);
    algo.AlgorithmChangeRun();

    edm4hep::SimCalorimeterHitCollection simhits;
    auto mhit = simhits.create(
      0xABABABAB, // std::uint64_t cellID
      1.0 /* GeV */, // float energy
      edm4hep::Vector3f({0. /* mm */, 0. /* mm */, 0. /* mm */}) // edm4hep::Vector3f position
    );
    mhit.addToContributions({
      0, // std::int32_t PDG
      0.5 /* GeV */, // float energy
      7.0 /* ns */, // float time
      {0. /* mm */, 0. /* mm */, 0. /* mm */}, // edm4hep::Vector3f stepPosition
    });
    mhit.addToContributions({
      0, // std::int32_t PDG
      0.5 /* GeV */, // float energy
      9.0 /* ns */, // float time
      {0. /* mm */, 0. /* mm */, 0. /* mm */}, // edm4hep::Vector3f stepPosition
    });

    std::unique_ptr<edm4hep::RawCalorimeterHitCollection> rawhits = algo.AlgorithmProcess(simhits);

    REQUIRE( (*rawhits).size() == 1 );
    REQUIRE( (*rawhits)[0].getCellID() == 0xABABABAB);
    REQUIRE( (*rawhits)[0].getAmplitude() == 123 + 111 );
    REQUIRE( (*rawhits)[0].getTimeStamp() == 7 ); // currently, earliest contribution is returned
  }
}
