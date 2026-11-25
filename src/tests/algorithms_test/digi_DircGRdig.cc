// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <podio/detail/Link.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

#include "algorithms/digi/DircGRdig.h"
#include "algorithms/digi/DircGRdigConfig.h"

namespace {

struct Out {
  edm4eic::RawTrackerHitCollection raw;
  edm4eic::MCRecoTrackerHitLinkCollection links;
  edm4eic::MCRecoTrackerHitAssociationCollection assocs;
};

// photons: (cellID, time [ns])
void run(const eicrecon::DircGRdigConfig& cfg,
         const std::vector<std::pair<std::uint64_t, double>>& photons, Out& out, int event = 1) {
  eicrecon::DircGRdig algo("DircGRdig_test");
  algo.applyConfig(cfg);
  algo.init();
  edm4hep::EventHeaderCollection headers;
  headers.create(event, 0);
  edm4hep::SimTrackerHitCollection sim;
  for (const auto& [id, t] : photons) {
    auto h = sim.create();
    h.setCellID(id);
    h.setTime(static_cast<float>(t));
  }
  algo.process({&headers, &sim}, {&out.raw, &out.links, &out.assocs});
}

} // namespace

TEST_CASE("DircGRdig: deadTime 0 makes every photon a hit, timeStamp = floor(t / LSB)",
          "[DircGRdig]") {
  eicrecon::DircGRdigConfig cfg; // defaults: deadTime 0, LSB 0.020 ns, no jitter
  Out out;
  run(cfg, {{7, 10.015}, {7, 10.005}, {9, 3.0}}, out);
  REQUIRE(out.raw.size() == 3);
  CHECK(out.links.size() == 3);
  CHECK(out.assocs.size() == 3);
  // sorted by cellID, then time
  CHECK(out.raw.at(0).getCellID() == 7);
  CHECK(out.raw.at(0).getTimeStamp() == static_cast<std::int32_t>(std::floor(10.005 / 0.020)));
  CHECK(out.raw.at(1).getTimeStamp() == static_cast<std::int32_t>(std::floor(10.015 / 0.020)));
  CHECK(out.raw.at(2).getCellID() == 9);
  CHECK(out.raw.at(2).getTimeStamp() == 150);
  for (const auto& h : out.raw) {
    CHECK(h.getCharge() == 1);
  }
}

TEST_CASE("DircGRdig: dead time keeps the earliest photon and counts merged ones", "[DircGRdig]") {
  eicrecon::DircGRdigConfig cfg;
  cfg.deadTime = 25.0;
  Out out;
  // pixel 5: 12 (first), 30 and 36.9 within 25 ns of 12 -> one hit with 3
  // photons; 37.0 is exactly t_hit + 25 -> new hit (dead time measured from
  // the hit start, not from the previous photon)
  run(cfg, {{5, 30.0}, {5, 12.0}, {5, 37.0}, {5, 36.9}}, out);
  REQUIRE(out.raw.size() == 2);
  CHECK(out.raw.at(0).getTimeStamp() == static_cast<std::int32_t>(std::floor(12.0 / 0.020)));
  CHECK(out.raw.at(0).getCharge() == 3);
  CHECK(out.raw.at(1).getTimeStamp() == static_cast<std::int32_t>(std::floor(37.0 / 0.020)));
  CHECK(out.raw.at(1).getCharge() == 1);
  // links: 3 + 1, the time-setting photon first, weight 1/n
  REQUIRE(out.links.size() == 4);
  CHECK(out.links.at(0).getFrom() == out.raw.at(0));
  CHECK(out.links.at(0).getTo().getTime() == 12.0F);
  CHECK_THAT(out.links.at(0).getWeight(), Catch::Matchers::WithinAbs(1.0 / 3, 1e-6));
  CHECK(out.links.at(3).getFrom() == out.raw.at(1));
}

TEST_CASE("DircGRdig: jitter smears times reproducibly per event", "[DircGRdig]") {
  eicrecon::DircGRdigConfig cfg;
  cfg.timeResolution = 0.001;
  cfg.timeJitter     = 0.020;
  std::vector<std::pair<std::uint64_t, double>> photons;
  for (int i = 0; i < 2000; i++) {
    photons.emplace_back(i, 10.0);
  }
  Out a;
  Out b;
  run(cfg, photons, a, 3);
  run(cfg, photons, b, 3);
  REQUIRE(a.raw.size() == photons.size());
  double s  = 0;
  double s2 = 0;
  for (std::size_t i = 0; i < a.raw.size(); i++) {
    CHECK(a.raw.at(i).getTimeStamp() == b.raw.at(i).getTimeStamp());
    const double t = (a.raw.at(i).getTimeStamp() + 0.5) * cfg.timeResolution - 10.0;
    s += t;
    s2 += t * t;
  }
  const double n    = a.raw.size();
  const double mean = s / n;
  const double rms  = std::sqrt(s2 / n - mean * mean);
  CHECK_THAT(mean, Catch::Matchers::WithinAbs(0.0, 0.002));
  CHECK_THAT(rms, Catch::Matchers::WithinAbs(0.020, 0.002));
}
