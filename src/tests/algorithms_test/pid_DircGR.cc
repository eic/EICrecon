// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <TFile.h>
#include <TObject.h>
#include <TParameter.h>
#include <TTree.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <gsl/pointers>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/pid/DircGR.h"
#include "algorithms/pid/DircGRConfig.h"
#include "services/dircgr/DircGRCalibSvc.h"
#include "services/dircgr/DircGRLUT.h"

namespace {

namespace fs = std::filesystem;

std::uint64_t dircCell(int module, int bar, int surface, int x = 0, int y = 0) {
  const auto* dec =
      algorithms::GeoSvc::instance().detector()->readout("MockDIRCHits").idSpec().decoder();
  std::uint64_t id = 0;
  dec->set(id, "module", module);
  dec->set(id, "surface", surface);
  dec->set(id, "bar", bar);
  dec->set(id, "x", x);
  dec->set(id, "y", y);
  return id;
}

struct LutPath {
  int ix, iy;
  double dx, dy, dz, t;
  double prob = 1.0;
};

// LUT grid = DIRCHits MCP cellID grid (ePIC)
constexpr int kNPixX = 107;
constexpr int kNPixY = 73;

// writes lut_bar{0..nfiles-1}.root into a fresh directory; file i gets the
// paths in `per_file[i]` and the grid parameters (nPixX, nPixY, nBar, bar = i)
fs::path writeLUT(const std::string& tag, const std::vector<std::vector<LutPath>>& per_file,
                  int nPixX = kNPixX, int nPixY = kNPixY) {
  fs::path dir = fs::temp_directory_path() / ("dircgr_test_" + tag);
  fs::remove_all(dir);
  fs::create_directories(dir);
  for (std::size_t i = 0; i < per_file.size(); i++) {
    TFile f((dir / ("lut_bar" + std::to_string(i) + ".root")).c_str(), "RECREATE");
    TTree t("dircLUT", "dircLUT");
    int ipixx   = 0;
    int ipixy   = 0;
    double dx   = 0;
    double dy   = 0;
    double dz   = 0;
    double time = 0;
    double prob = 0;
    t.Branch("ipixx", &ipixx);
    t.Branch("ipixy", &ipixy);
    t.Branch("OPdirx", &dx);
    t.Branch("OPdiry", &dy);
    t.Branch("OPdirz", &dz);
    t.Branch("OPtime", &time);
    t.Branch("OPprob", &prob);
    for (const auto& p : per_file[i]) {
      ipixx = p.ix;
      ipixy = p.iy;
      dx    = p.dx;
      dy    = p.dy;
      dz    = p.dz;
      time  = p.t;
      prob  = p.prob;
      t.Fill();
    }
    t.Write();
    TParameter<int>("nPixX", nPixX).Write();
    TParameter<int>("nPixY", nPixY).Write();
    TParameter<int>("nBar", static_cast<int>(per_file.size())).Write();
    TParameter<int>("bar", static_cast<int>(i)).Write();
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

TEST_CASE("DircGR LUT loader maps file lut_bar{i} to bar i", "[DircGR]") {
  auto dir = writeLUT(
      "barmap", {{{.ix = 5, .iy = 7, .dx = 0.1, .dy = 0.2, .dz = -0.9, .t = 1.0}},
                 {{.ix = 5, .iy = 7, .dx = 0.3, .dy = 0.4, .dz = -0.8, .t = 2.0, .prob = 0.25}}});
  auto& svc       = eicrecon::DircGRCalibSvc::instance();
  const auto* lut = svc.lut(dir.string());
  CHECK(svc.lut(dir.string()) == lut); // loaded once, shared
  CHECK(lut->nPixX() == kNPixX);       // grid from the files
  CHECK(lut->nPixY() == kNPixY);
  CHECK(lut->nBar() == 2);
  REQUIRE(lut->nPathsTotal() == 2);
  REQUIRE(lut->paths(5, 7, 0).size() == 1);
  CHECK(lut->paths(5, 7, 0)[0].time == 1.0); // from file 0
  CHECK(lut->paths(5, 7, 0)[0].dir.z == -0.9);
  CHECK(lut->paths(5, 7, 1)[0].time == 2.0); // from file 1
  CHECK(lut->paths(5, 7, 1)[0].prob == 0.25);
  CHECK(lut->paths(4, 7, 0).empty());   // empty pixel
  CHECK(lut->paths(200, 7, 0).empty()); // outside the grid
  CHECK(lut->paths(5, 7, 2).empty());   // no such bar
}

TEST_CASE("DircGR LUT loader rejects inconsistent or incomplete files", "[DircGR]") {
  const LutPath p{.ix = 5, .iy = 7, .dx = 0, .dy = 0, .dz = -1, .t = 1.0};
  SECTION("grid differs between files") {
    auto dir = writeLUT("badgrid", {{p}, {p}});
    {
      TFile f((dir / "lut_bar1.root").c_str(), "UPDATE");
      TParameter<int>("nPixX", 50).Write("nPixX", TObject::kOverwrite);
    }
    REQUIRE_THROWS(eicrecon::DircGRLUT(dir.string()));
  }
  SECTION("pixel outside the grid") {
    auto dir = writeLUT("smallgrid", {{p}}, 4, 4);
    REQUIRE_THROWS(eicrecon::DircGRLUT(dir.string()));
  }
  SECTION("missing bar file") {
    auto dir = writeLUT("missing", {{p}, {p}});
    fs::remove(dir / "lut_bar1.root");
    REQUIRE_THROWS(eicrecon::DircGRLUT(dir.string()));
  }
}

TEST_CASE("DircGR static helpers", "[DircGR]") {
  using eicrecon::DircGR;
  eicrecon::DircGRTrackGates g;
  g.cosAngLower = -0.8;
  g.cosAngUpper = -0.6;
  CHECK(DircGR::cosAngOK(-0.7, -1, g)); // direct: cos itself
  CHECK_FALSE(DircGR::cosAngOK(0.7, -1, g));
  CHECK(DircGR::cosAngOK(0.7, +1, g)); // via mirror: -cos
  CHECK_FALSE(DircGR::cosAngOK(-0.7, 0, g));
}

TEST_CASE("DircGR time gate results", "[DircGR]") {
  using TG                   = eicrecon::DircGR::TimeGate;
  eicrecon::DircGRConfig cfg = baseConfig(); // half width 1 ns
  eicrecon::DircGR algo("DircGR_test_gate");
  algo.applyConfig(cfg);
  eicrecon::DircGRTrackGates g;
  g.use1    = true;
  g.use2    = true;
  g.tbeg1OP = 5.0;
  g.tbeg2OP = 15.0;
  // expected times: direct 10 ns, via mirror 30 ns
  CHECK(algo.timeGateTight(10.3, 10.0, 30.0, g) == TG::Direct);
  CHECK(algo.timeGateTight(29.5, 10.0, 30.0, g) == TG::ViaMirror);
  CHECK(algo.timeGateTight(20.0, 10.0, 30.0, g) == TG::NoMatch);
  g.tbeg2OP = 5.0; // both match (and both late enough): mirror wins
  CHECK(algo.timeGateTight(10.0, 10.0, 10.5, g) == TG::ViaMirror);
  g.use2 = false;
  CHECK(algo.timeGateTight(10.0, 10.0, 10.5, g) == TG::Direct);
  g.tbeg1OP = 12.0; // hit earlier than the earliest allowed direct photon
  CHECK(algo.timeGateTight(10.3, 10.0, 30.0, g) == TG::DirectTooEarly);
  g.use2    = true;
  g.tbeg2OP = 31.0;
  CHECK(algo.timeGateTight(30.2, 10.0, 30.0, g) == TG::MirrorTooEarly);
}

TEST_CASE("DircGR reconstructs a synthetic pion ring", "[DircGR]") {
  // ---- track: 3 GeV pi+ entering bar 0 of box 0 at x = 770 mm, z = -1000 mm,
  // moving radially outward from the origin (detector theta ~128 deg, so
  // only "direct" photons towards the prism are allowed)
  const double p      = 3.0;
  const double mpi    = 0.13957039;
  const double n      = 1.4738;
  const double xin    = 770.0;
  const double zin    = -1000.0;
  const double tin    = 4.0;
  const double r      = std::hypot(xin, zin);
  const double ux     = xin / r;
  const double uz     = zin / r; // uy = 0
  const double beta   = p / std::sqrt(p * p + mpi * mpi);
  const double thetaC = std::acos(1.0 / (n * beta)); // expected pion angle

  // ---- one LUT path: direction d with cos(polar) in the middle of the
  // cos(angle) envelope (beta = 1 cone around the track), and angle thetaC
  // to the track
  const double thC1 = std::acos(1.0 / n);
  const double c    = 0.5 * (std::cos(std::acos(uz) - thC1) + std::cos(std::acos(uz) + thC1));
  const double sa   = std::sqrt(1 - c * c);
  const double cosb = (std::cos(thetaC) - c * uz) / (sa * ux);
  REQUIRE(std::abs(cosb) <= 1.0);
  const double dx = sa * cosb;
  const double dy = sa * std::sqrt(1 - cosb * cosb);
  const double dz = c;
  // the x-flipped ambiguity must not be close to any hypothesis (else the
  // per-path hypothesis choice could pick it)
  const double thx = std::acos(-dx * ux + dz * uz);
  for (double m : {0.13957039, 0.493677, 0.93827208}) {
    const double b = p / std::sqrt(p * p + m * m);
    REQUIRE(std::abs(thx - std::acos(1.0 / (n * b))) > 0.02);
  }

  // LUT pixel (ix, iy) = MCP cell (x, y) = (iy - 36, 53 - ix); hit time =
  // expected direct arrival time
  eicrecon::DircGRConfig cfg = baseConfig();
  const int ix               = 50;
  const int iy               = 30;
  const int cellX            = iy - (kNPixY - 1) / 2;
  const int cellY            = (kNPixX - 1) / 2 - ix;
  const double tprism        = 1.5; // [ns] LUT time in the prism
  const double texp          = tprism + (zin - cfg.zBarMin) / std::abs(c) / cfg.vBar + tin;
  cfg.lutPath =
      writeLUT("ring", {{{.ix = ix, .iy = iy, .dx = dx, .dy = dy, .dz = dz, .t = tprism}}})
          .string();

  eicrecon::DircGR algo("DircGR_test");
  algo.level(algorithms::LogLevel::kInfo);
  algo.applyConfig(cfg);
  algo.init();

  auto segments = std::make_unique<edm4eic::TrackSegmentCollection>();
  auto seg      = segments->create();
  edm4eic::TrackPoint pt;
  pt.surface  = dircCell(0, 0, 1);
  pt.position = {static_cast<float>(xin), 0.F, static_cast<float>(zin)};
  pt.momentum = {static_cast<float>(p * ux), 0.F, static_cast<float>(p * uz)};
  pt.time     = static_cast<float>(tin);
  seg.addToPoints(pt);
  segments->create(); // second segment without points: must still get an output

  auto run = [&](double dt, int module) {
    auto hits = std::make_unique<edm4eic::RawTrackerHitCollection>();
    for (int i = 0; i < 3; i++) { // 3 raw hits in the same pixel (minPhotons = 3)
      auto h = hits->create();
      h.setCellID(dircCell(module, 0, 0, cellX, cellY));
      h.setTimeStamp(static_cast<std::int32_t>(std::floor((texp + dt) / cfg.timeResolution)));
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
  pt.position = {770.F, 0.F, -1000.F};
  pt.momentum = {2.F, 0.F, -2.F};
  seg.addToPoints(pt);
  auto hits    = std::make_unique<edm4eic::RawTrackerHitCollection>();
  auto pids    = std::make_unique<edm4eic::CherenkovParticleIDCollection>();
  auto radinfo = std::make_unique<edm4eic::IrtRadiatorInfoCollection>();
  algo.process({segments.get(), hits.get()}, {pids.get(), radinfo.get()});
  REQUIRE(pids->size() == 1);
  CHECK(pids->at(0).getNpe() == 0);
  for (const auto& h : pids->at(0).getHypotheses()) {
    CHECK(h.weight == 0);
  }
}
