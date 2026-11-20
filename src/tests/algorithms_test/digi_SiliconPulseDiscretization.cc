// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <catch2/catch_test_macros.hpp>
#include <edm4eic/SimPulseCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <vector>

#include "algorithms/digi/SiliconPulseDiscretization.h"
#include "algorithms/digi/SiliconPulseDiscretizationConfig.h"

namespace {

// Leaf pulse as made by PulseGeneration: one sim hit, ten samples
edm4eic::MutableSimPulse makeLeaf(edm4eic::SimPulseCollection& leaves,
                                  edm4hep::SimTrackerHitCollection& sim_hits, std::uint64_t cellID,
                                  double time) {
  auto sim_hit = sim_hits.create();
  sim_hit.setCellID(cellID);
  sim_hit.setTime(time);
  sim_hit.setEDep(1. * edm4eic::unit::keV);
  auto leaf = leaves.create();
  leaf.setCellID(cellID);
  leaf.setTime(time);
  leaf.setInterval(0.5 * edm4eic::unit::ns);
  for (float amplitude : {1, 2, 3, 4, 5, 5, 4, 3, 2, 1}) {
    leaf.addToAmplitude(-amplitude);
  }
  leaf.addToTrackerHits(sim_hit);
  return leaf;
}

// Combined pulse as made by PulseCombiner from time-ordered leaves on one cell
void combine(edm4eic::SimPulseCollection& combined, const std::vector<edm4eic::SimPulse>& leaves) {
  auto pulse = combined.create();
  pulse.setCellID(leaves.front().getCellID());
  pulse.setTime(leaves.front().getTime());
  pulse.setInterval(leaves.front().getInterval());
  std::vector<float> amplitude;
  for (const auto& leaf : leaves) {
    auto start = static_cast<std::size_t>(
        std::round((leaf.getTime() - pulse.getTime()) / pulse.getInterval()));
    amplitude.resize(std::max(amplitude.size(), start + leaf.getAmplitude().size()), 0);
    for (std::size_t i = 0; i < leaf.getAmplitude().size(); ++i) {
      amplitude[start + i] += leaf.getAmplitude(i);
    }
    pulse.addToPulses(leaf);
    for (const auto& hit : leaf.getTrackerHits()) {
      pulse.addToTrackerHits(hit);
    }
  }
  for (float value : amplitude) {
    pulse.addToAmplitude(value);
  }
}

// Sim hits recorded in the truth entry aligned with the time series of (cellID, time)
std::optional<std::vector<edm4hep::SimTrackerHit>>
truthHits(const edm4hep::RawTimeSeriesCollection& series, const edm4eic::SimPulseCollection& truth,
          std::uint64_t cellID, double time) {
  for (std::size_t i = 0; i < series.size(); ++i) {
    if (series[i].getCellID() == cellID && std::abs(series[i].getTime() - time) < 1e-3) {
      auto hits = truth[i].getTrackerHits();
      return std::vector<edm4hep::SimTrackerHit>(hits.begin(), hits.end());
    }
  }
  return std::nullopt;
}

} // namespace

TEST_CASE("the SiliconPulseDiscretization keeps truth per time series",
          "[SiliconPulseDiscretization]") {
  eicrecon::SiliconPulseDiscretization algo("SiliconPulseDiscretization");
  eicrecon::SiliconPulseDiscretizationConfig cfg;
  cfg.EICROC_period = 25 * edm4eic::unit::ns;
  cfg.local_period  = 25 * edm4eic::unit::ns / 1024;

  edm4hep::SimTrackerHitCollection sim_hits;
  edm4eic::SimPulseCollection leaves;
  edm4eic::SimPulseCollection combined;

  SECTION("leaves of a combined pulse go to the cycle they lie in") {
    cfg.global_offset = 0;
    algo.applyConfig(cfg);
    algo.init();

    // cell 1: two leaves in cycles 0 and 1, combined into one pulse
    auto leaf_a = makeLeaf(leaves, sim_hits, 1, 2 * edm4eic::unit::ns);
    auto leaf_b = makeLeaf(leaves, sim_hits, 1, 30 * edm4eic::unit::ns);
    combine(combined, {leaf_a, leaf_b});
    // cell 2: a single uncombined pulse (a clone, so it is its own leaf) in cycle 2
    auto leaf_d = makeLeaf(combined, sim_hits, 2, 51 * edm4eic::unit::ns);
    // cell 3: leaves in cycles 0 and 2; cycle 1 is covered only by the zero-filled gap
    auto leaf_e = makeLeaf(leaves, sim_hits, 3, 2 * edm4eic::unit::ns);
    auto leaf_f = makeLeaf(leaves, sim_hits, 3, 57 * edm4eic::unit::ns);
    combine(combined, {leaf_e, leaf_f});

    edm4hep::RawTimeSeriesCollection series;
    edm4eic::SimPulseCollection truth;
    algo.process({&combined}, {&series, &truth});

    REQUIRE(series.size() == 6);
    REQUIRE(truth.size() == series.size());
    for (std::size_t i = 0; i < series.size(); ++i) {
      REQUIRE(truth[i].getCellID() == series[i].getCellID());
      REQUIRE(truth[i].getTime() == series[i].getTime());
      REQUIRE(truth[i].pulses_size() >= 1);
      REQUIRE(truth[i].trackerHits_size() >= 1);
    }

    using Hits = std::vector<edm4hep::SimTrackerHit>;
    auto hit   = [](const edm4eic::SimPulse& leaf) { return leaf.getTrackerHits(0); };
    REQUIRE(truthHits(series, truth, 1, 0) == Hits{hit(leaf_a)});
    REQUIRE(truthHits(series, truth, 1, 25) == Hits{hit(leaf_b)});
    REQUIRE(truthHits(series, truth, 2, 50) == Hits{hit(leaf_d)});
    REQUIRE(truthHits(series, truth, 3, 0) == Hits{hit(leaf_e)});
    // the gap cycle falls back to all leaves of the pulse that made it active
    REQUIRE(truthHits(series, truth, 3, 25) == Hits{hit(leaf_e), hit(leaf_f)});
    REQUIRE(truthHits(series, truth, 3, 50) == Hits{hit(leaf_f)});
  }

  SECTION("the global offset moves a leaf into the earlier cycle's time series") {
    cfg.global_offset = -1 * edm4eic::unit::ns;
    algo.applyConfig(cfg);
    algo.init();

    // leaf_b starts at 25.5 ns, but its shifted samples begin at 24.5 ns in cycle 0
    auto leaf_a = makeLeaf(leaves, sim_hits, 1, 2 * edm4eic::unit::ns);
    auto leaf_b = makeLeaf(leaves, sim_hits, 1, 25.5 * edm4eic::unit::ns);
    combine(combined, {leaf_a, leaf_b});

    edm4hep::RawTimeSeriesCollection series;
    edm4eic::SimPulseCollection truth;
    algo.process({&combined}, {&series, &truth});

    using Hits = std::vector<edm4hep::SimTrackerHit>;
    auto hit   = [](const edm4eic::SimPulse& leaf) { return leaf.getTrackerHits(0); };
    REQUIRE(truth.size() == series.size());
    REQUIRE(truthHits(series, truth, 1, 0) == Hits{hit(leaf_a), hit(leaf_b)});
    REQUIRE(truthHits(series, truth, 1, 25) == Hits{hit(leaf_b)});
  }
}
