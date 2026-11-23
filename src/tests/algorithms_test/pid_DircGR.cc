// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <TFile.h>
#include <TTree.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <algorithms/logger.h>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/pid/DircGR.h"
#include "algorithms/pid/DircGRConfig.h"
#include "algorithms/pid/DircGRCosAngTables.h"
#include "algorithms/pid/DircGRLUT.h"

namespace {

namespace fs = std::filesystem;

std::uint64_t dircCell(int module, int bar, int surface) {
  const auto* dec =
      algorithms::GeoSvc::instance().detector()->readout("MockDIRCHits").idSpec().decoder();
  std::uint64_t id = 0;
  dec->set(id, "module", module);
  dec->set(id, "surface", surface);
  dec->set(id, "bar", bar);
  return id;
}

struct LutPath {
  int ix, iy;
  double dx, dy, dz, t;
};

// writes lut_bar{0..nfiles-1}.root into a fresh directory; file i gets the
// paths in `per_file[i]`
fs::path writeLUT(const std::string& tag, const std::vector<std::vector<LutPath>>& per_file) {
  fs::path dir = fs::temp_directory_path() / ("dircgr_test_" + tag);
  fs::remove_all(dir);
  fs::create_directories(dir);
  for (std::size_t i = 0; i < per_file.size(); i++) {
    TFile f((dir / ("lut_bar" + std::to_string(i) + ".root")).c_str(), "RECREATE");
    TTree t("dircLUT", "dircLUT");
    int ipixx = 0, ipixy = 0;
    double prob = 1, dx = 0, dy = 0, dz = 0, time = 0;
    t.Branch("ipixx", &ipixx);
    t.Branch("ipixy", &ipixy);
    t.Branch("OPprob", &prob);
    t.Branch("OPdirx", &dx);
    t.Branch("OPdiry", &dy);
    t.Branch("OPdirz", &dz);
    t.Branch("OPtime", &time);
    for (const auto& p : per_file[i]) {
      ipixx = p.ix;
      ipixy = p.iy;
      dx    = p.dx;
      dy    = p.dy;
      dz    = p.dz;
      time  = p.t;
      t.Fill();
    }
    t.Write();
  }
  return dir;
}

eicrecon::DircGRConfig baseConfig() {
  eicrecon::DircGRConfig cfg;
  cfg.readout     = "MockDIRCHits";
  cfg.chrcorrFile = ""; // no chromatic correction
  return cfg;
}

} // namespace

TEST_CASE("DircGR LUT loader maps file lut_bar{i} to bar nBar-1-i", "[DircGR]") {
  // 2 bars: file 0 -> bar 1, file 1 -> bar 0
  auto dir = writeLUT("barmap", {{{5, 7, 0.1, 0.2, -0.9, 1.0}}, {{5, 7, 0.3, 0.4, -0.8, 2.0}}});
  auto lut = eicrecon::DircGRLUT::get(dir.string(), 108, 73, 2);
  REQUIRE(lut->nPathsTotal() == 2);
  REQUIRE(lut->paths(5, 7, 1).size() == 1);
  CHECK(lut->paths(5, 7, 1).time[0] == 1.0); // from file 0
  CHECK(lut->paths(5, 7, 0).time[0] == 2.0); // from file 1
  CHECK(lut->paths(4, 7, 0).size() == 0);    // empty pixel
  CHECK(lut->paths(200, 7, 0).size() == 0);  // outside the grid
}

TEST_CASE("DircGR static helpers", "[DircGR]") {
  using eicrecon::DircGR;
  CHECK(DircGR::kzFromZ(0.0) == eicrecon::dircgr::kCosAngOffset);
  CHECK(DircGR::kzFromZ(-1.0e6) == 0);
  CHECK(DircGR::kzFromZ(1.0e6) == eicrecon::dircgr::kNCosAng - 1);

  eicrecon::DircGRTrackGates g;
  g.cosAngLower = -0.8;
  g.cosAngUpper = -0.6;
  CHECK(DircGR::cosAngOK(-0.7, -1, g)); // direct: cos itself
  CHECK_FALSE(DircGR::cosAngOK(0.7, -1, g));
  CHECK(DircGR::cosAngOK(0.7, +1, g)); // via mirror: -cos
  CHECK_FALSE(DircGR::cosAngOK(-0.7, 0, g));
}

TEST_CASE("DircGR reconstructs a synthetic pion ring", "[DircGR]") {
  // ---- track: 3 GeV pi+ entering bar 0 of box 0 at x = 770 mm, z = -1000 mm,
  // moving radially outward from the origin (detector theta ~128 deg, so
  // only "direct" photons towards the prism are allowed)
  const double p = 3.0, mpi = 0.13957039, n = 1.4738;
  const double xin = 770.0, zin = -1000.0, tin = 4.0;
  const double r  = std::hypot(xin, zin);
  const double ux = xin / r, uz = zin / r; // uy = 0
  const double beta   = p / std::sqrt(p * p + mpi * mpi);
  const double thetaC = std::acos(1.0 / (n * beta)); // expected pion angle

  // ---- one LUT path: direction d with cos(polar) in the middle of the
  // cos(angle) envelope at this z, and angle thetaC to the track
  const int kz    = eicrecon::DircGR::kzFromZ(zin);
  const double c  = 0.5 * (eicrecon::dircgr::kCosAngLower[kz] + eicrecon::dircgr::kCosAngUpper[kz]);
  const double sa = std::sqrt(1 - c * c);
  const double cosb = (std::cos(thetaC) - c * uz) / (sa * ux);
  REQUIRE(std::abs(cosb) <= 1.0);
  const double dx = sa * cosb, dy = sa * std::sqrt(1 - cosb * cosb), dz = c;
  // the x-flipped ambiguity must not be close to any hypothesis (else the
  // per-path hypothesis choice could pick it)
  const double thx = std::acos(-dx * ux + dz * uz);
  for (double m : {0.13957039, 0.493677, 0.93827208}) {
    const double b = p / std::sqrt(p * p + m * m);
    REQUIRE(std::abs(thx - std::acos(1.0 / (n * b))) > 0.02);
  }

  // pixel (ix, iy) and its centre in global coordinates (box 0: locx = -y,
  // locy = x); hit time = expected direct arrival time
  eicrecon::DircGRConfig cfg = baseConfig();
  cfg.nBar                   = 1;
  const int ix = 50, iy = 30;
  const double locx   = cfg.locXmin + (ix + 0.5) * cfg.pixelSize;
  const double locy   = cfg.locYmin + (iy + 0.5) * cfg.pixelSize;
  const double tprism = 1.5; // [ns] LUT time in the prism
  const double texp   = tprism + (zin - cfg.zBarMin) / std::abs(c) / cfg.vBar + tin;
  cfg.lutPath         = writeLUT("ring", {{{ix, iy, dx, dy, dz, tprism}}}).string();

  eicrecon::DircGR algo("DircGR_test");
  algo.level(algorithms::LogLevel::kInfo);
  algo.applyConfig(cfg);
  algo.init();

  auto segments = std::make_unique<edm4eic::TrackSegmentCollection>();
  auto seg      = segments->create();
  edm4eic::TrackPoint pt;
  pt.surface  = dircCell(0, 0, 1);
  pt.position = {static_cast<float>(xin), 0.f, static_cast<float>(zin)};
  pt.momentum = {static_cast<float>(p * ux), 0.f, static_cast<float>(p * uz)};
  pt.time     = static_cast<float>(tin);
  seg.addToPoints(pt);
  segments->create(); // second segment without points: must still get an output

  auto run = [&](double dt, int module) {
    auto hits = std::make_unique<edm4hep::SimTrackerHitCollection>();
    for (int i = 0; i < 3; i++) { // 3 photons in the same pixel (minPhotons = 3)
      auto h = hits->create();
      h.setCellID(dircCell(module, 0, 0));
      // rotate the box-0 position into box `module`
      const double phi = module * 2 * M_PI / cfg.nBox;
      const double x0 = locy, y0 = -locx;
      h.setPosition({x0 * std::cos(phi) - y0 * std::sin(phi),
                     x0 * std::sin(phi) + y0 * std::cos(phi), cfg.zReadout});
      h.setTime(static_cast<float>(texp + dt));
    }
    auto pids    = std::make_unique<edm4eic::CherenkovParticleIDCollection>();
    auto radinfo = std::make_unique<edm4eic::IrtRadiatorInfoCollection>();
    algo.process({segments.get(), hits.get()}, {pids.get(), radinfo.get()});
    return std::make_pair(std::move(pids), std::move(radinfo));
  };

  SECTION("in time, same box: pion ring reconstructed") {
    auto [pids, radinfo] = run(0.0, 0);
    REQUIRE(pids->size() == 2); // index-aligned with the segments
    REQUIRE(radinfo->size() == 2);
    CHECK(pids->at(0).getChargedParticle() == segments->at(0));
    CHECK(pids->at(0).getNpe() == 3);
    CHECK(radinfo->at(0).getNpe() == 3);
    // 1 mrad histogram bins: within half a bin of the constructed angle
    CHECK_THAT(radinfo->at(0).getAngle(), Catch::Matchers::WithinAbs(1000. * thetaC, 0.5));
    REQUIRE(pids->at(0).getHypotheses().size() == 3);
    float wbest = -1e30;
    int best    = 0;
    for (const auto& h : pids->at(0).getHypotheses()) {
      if (h.weight > wbest) {
        wbest = h.weight;
        best  = h.PDG;
      }
    }
    CHECK(best == 211);
    CHECK(pids->at(0).getThetaPhiPhotons().size() == 3);
    // segment without points: placeholder output, no PID
    CHECK(pids->at(1).getNpe() == 0);
  }

  SECTION("photons outside the time gate are not used") {
    auto [pids, radinfo] = run(5.0, 0);
    CHECK(pids->at(0).getNpe() == 0);
    CHECK(radinfo->at(0).getAngle() == 0);
  }

  SECTION("photons in another bar box are not used") {
    auto [pids, radinfo] = run(0.0, 1);
    CHECK(pids->at(0).getNpe() == 0);
    CHECK(radinfo->at(0).getNhits() == 0);
  }
}

TEST_CASE("DircGR without LUT runs and gives no PID", "[DircGR]") {
  eicrecon::DircGRConfig cfg = baseConfig();
  cfg.lutPath                = (fs::temp_directory_path() / "dircgr_test_does_not_exist").string();
  eicrecon::DircGR algo("DircGR_test_nolut");
  algo.applyConfig(cfg);
  REQUIRE_NOTHROW(algo.init());

  auto segments = std::make_unique<edm4eic::TrackSegmentCollection>();
  auto seg      = segments->create();
  edm4eic::TrackPoint pt;
  pt.surface  = dircCell(0, 0, 1);
  pt.position = {770.f, 0.f, -1000.f};
  pt.momentum = {2.f, 0.f, -2.f};
  seg.addToPoints(pt);
  auto hits    = std::make_unique<edm4hep::SimTrackerHitCollection>();
  auto pids    = std::make_unique<edm4eic::CherenkovParticleIDCollection>();
  auto radinfo = std::make_unique<edm4eic::IrtRadiatorInfoCollection>();
  algo.process({segments.get(), hits.get()}, {pids.get(), radinfo.get()});
  REQUIRE(pids->size() == 1);
  CHECK(pids->at(0).getNpe() == 0);
  for (const auto& h : pids->at(0).getHypotheses()) {
    CHECK(h.weight == 0);
  }
}
