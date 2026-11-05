// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Evaluator/DD4hepUnits.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/detail/Link.h>
#include <cstdint>
#include <cstdlib>
#include <deque>

#include "algorithms/digi/CFDROCDigitization.h"
#include "algorithms/digi/CFDROCDigitizationConfig.h"

TEST_CASE("the CFDROC digitization links raw hits to sim hits", "[CFDROCDigitization]") {
  eicrecon::CFDROCDigitization algo("CFDROCDigitization");

  eicrecon::CFDROCDigitizationConfig cfg;
  algo.applyConfig(cfg);
  algo.init();

  const std::uint64_t cellID_a = 1;
  const std::uint64_t cellID_b = 2;

  edm4hep::SimTrackerHitCollection sim_hits_coll;
  auto sim_hit_a1 = sim_hits_coll.create();
  sim_hit_a1.setCellID(cellID_a);
  sim_hit_a1.setEDep(2. * dd4hep::keV);
  auto sim_hit_a2 = sim_hits_coll.create();
  sim_hit_a2.setCellID(cellID_a);
  sim_hit_a2.setEDep(6. * dd4hep::keV);
  auto sim_hit_b = sim_hits_coll.create();
  sim_hit_b.setCellID(cellID_b);
  sim_hit_b.setEDep(5. * dd4hep::keV);

  // Triangular waveforms, one per cell. A strictly monotonic rise avoids the
  // integer plateaus that the CFD peak finder would treat as extra peaks.
  edm4hep::RawTimeSeriesCollection time_series_coll;
  edm4eic::SimPulseCollection truth_coll;
  auto add_waveform = [&](std::uint64_t cellID, double time, int peak) {
    auto pulse = time_series_coll.create();
    pulse.setCellID(cellID);
    pulse.setTime(time);
    pulse.setInterval(cfg.tMax / cfg.tdc_range);
    const int peak_bin   = cfg.tdc_range / 2;
    const int half_width = peak / 2;
    for (int i = 0; i < cfg.tdc_range; ++i) {
      int distance = std::abs(i - peak_bin);
      pulse.addToAdcCounts(distance < half_width ? -(peak - 2 * distance) : 0);
    }
    auto truth = truth_coll.create();
    truth.setCellID(cellID);
    truth.setTime(time);
    return truth;
  };
  auto truth_a = add_waveform(cellID_a, 0., 200);
  truth_a.addToTrackerHits(sim_hit_a1);
  truth_a.addToTrackerHits(sim_hit_a2);
  auto truth_b = add_waveform(cellID_b, cfg.tMax, 100);
  truth_b.addToTrackerHits(sim_hit_b);

  SECTION("each raw hit links to the sim hits of its waveform, weighted by EDep") {
    edm4eic::RawTrackerHitCollection rawhits_coll;
    edm4eic::MCRecoTrackerHitLinkCollection links_coll;
    edm4eic::MCRecoTrackerHitAssociationCollection assocs_coll;
    algo.process({&time_series_coll, &truth_coll}, {&rawhits_coll, &links_coll, &assocs_coll});

    REQUIRE(rawhits_coll.size() == 2);
    REQUIRE(rawhits_coll[0].getCellID() == cellID_a);
    REQUIRE(rawhits_coll[1].getCellID() == cellID_b);
    REQUIRE(rawhits_coll[1].getTimeStamp() >= cfg.tdc_range);

    REQUIRE(links_coll.size() == 3);
    REQUIRE(assocs_coll.size() == 3);
    REQUIRE(links_coll[0].getFrom() == rawhits_coll[0]);
    REQUIRE(links_coll[0].getTo() == sim_hit_a1);
    REQUIRE_THAT(links_coll[0].getWeight(), Catch::Matchers::WithinAbs(0.25, 1e-6));
    REQUIRE(links_coll[1].getFrom() == rawhits_coll[0]);
    REQUIRE(links_coll[1].getTo() == sim_hit_a2);
    REQUIRE_THAT(links_coll[1].getWeight(), Catch::Matchers::WithinAbs(0.75, 1e-6));
    REQUIRE(links_coll[2].getFrom() == rawhits_coll[1]);
    REQUIRE(links_coll[2].getTo() == sim_hit_b);
    REQUIRE_THAT(links_coll[2].getWeight(), Catch::Matchers::WithinAbs(1.0, 1e-6));
    for (std::size_t i = 0; i < links_coll.size(); ++i) {
      REQUIRE(assocs_coll[i].getRawHit() == links_coll[i].getFrom());
      REQUIRE(assocs_coll[i].getSimHit() == links_coll[i].getTo());
      REQUIRE(assocs_coll[i].getWeight() == links_coll[i].getWeight());
    }
  }

  SECTION("misaligned truth gives raw hits without links") {
    truth_coll.create();
    edm4eic::RawTrackerHitCollection rawhits_coll;
    edm4eic::MCRecoTrackerHitLinkCollection links_coll;
    edm4eic::MCRecoTrackerHitAssociationCollection assocs_coll;
    algo.process({&time_series_coll, &truth_coll}, {&rawhits_coll, &links_coll, &assocs_coll});

    REQUIRE(rawhits_coll.size() == 2);
    REQUIRE(links_coll.empty());
    REQUIRE(assocs_coll.empty());
  }
}
