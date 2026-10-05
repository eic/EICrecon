// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/geo.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/unit_system.h>
#include <gsl/pointers>
#include <string>

#include "algorithms/tracking/TrackerHitReconstruction.h"
#include "algorithms/tracking/TrackerHitReconstructionConfig.h"

using eicrecon::TrackerHitReconstruction;
using eicrecon::TrackerHitReconstructionConfig;

namespace {

// Reuse the mock MPGD geometry, which is registered in the VolumeManager so that the
// CellIDPositionConverter can resolve positions and cell dimensions.
// system=3 matches the mock geometry's addPhysVolID("system", 3); strip=1 is the p-strip
// sensor with a 1 mm CartesianGridXY segmentation.
dd4hep::DDSegmentation::CellID makeCellID(int x, int y) {
  auto id_desc = algorithms::GeoSvc::instance().detector()->readout("MockMPGDHits").idSpec();
  auto encoder = id_desc.decoder();
  dd4hep::DDSegmentation::CellID cid = 0;
  encoder->set(cid, "system", 3);
  encoder->set(cid, "layer", 0);
  encoder->set(cid, "module", 0);
  encoder->set(cid, "strip", 1);
  encoder->set(cid, "x", x);
  encoder->set(cid, "y", y);
  return cid;
}

} // namespace

TEST_CASE("TrackerHitReconstruction: empty input produces empty output",
          "[TrackerHitReconstruction]") {
  TrackerHitReconstruction algo("test_reco_empty");
  TrackerHitReconstructionConfig cfg;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;
  algo.process({&raw_hits}, {&rec_hits});
  REQUIRE(rec_hits.empty());
}

TEST_CASE("TrackerHitReconstruction: time and time error come from timestamp and config",
          "[TrackerHitReconstruction]") {
  TrackerHitReconstruction algo("test_reco_time");
  TrackerHitReconstructionConfig cfg;
  cfg.timeResolution = 25 * edm4eic::unit::ps;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  const auto cellID = makeCellID(2, -3);
  raw_hits.create(cellID, /*charge=*/500, /*timeStamp=*/123456); // ps

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  const auto& hit = rec_hits[0];
  CHECK(hit.getCellID() == cellID);
  CHECK(hit.getTime() == Catch::Approx(123.456));
  CHECK(hit.getTimeError() == Catch::Approx(0.025));
  CHECK(hit.getEdep() == Catch::Approx(500 / 1.0e6));
  CHECK(hit.getRawHit().getCellID() == cellID);
}

TEST_CASE("TrackerHitReconstruction: position variance is the cell size squared over 12",
          "[TrackerHitReconstruction]") {
  TrackerHitReconstruction algo("test_reco_position");
  TrackerHitReconstructionConfig cfg;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;
  raw_hits.create(makeCellID(0, 0), 100, 0);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  // 1 mm pitch in both local coordinates
  CHECK(rec_hits[0].getPositionError().xx == Catch::Approx(1.0 / 12.0));
  CHECK(rec_hits[0].getPositionError().yy == Catch::Approx(1.0 / 12.0));
}

TEST_CASE("TrackerHitReconstruction: default time error is 10 ns", "[TrackerHitReconstruction]") {
  TrackerHitReconstruction algo("test_reco_default");
  TrackerHitReconstructionConfig cfg;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;
  raw_hits.create(makeCellID(1, 1), 100, 0);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  CHECK(rec_hits[0].getTimeError() == Catch::Approx(10 * edm4eic::unit::ns));
}
