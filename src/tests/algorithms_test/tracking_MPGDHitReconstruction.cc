// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

// Unit tests for the MPGDHitReconstruction algorithm.
//
// The mock detector in algorithmsInit.cc provides a "MockMPGDHits" readout
// with full programmatic DD4hep geometry (world volume, sensitive volume,
// VolumeManager population). This enables testing both init() and process().
//
// MockMPGDHits IDDescriptor:
//   "system:8,layer:4,module:12,strip:28:4,x:32:-16,y:-16"
//   system=3, layer=0, module=0 are encoded in the placed volume IDs.
//   strip=1 → p-strip (coordinate in x field at offset 32)
//   strip=2 → n-strip (coordinate in y field at offset 48)

#include <DD4hep/BitFieldCoder.h>
#include <DD4hep/IDDescriptor.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>

#include "algorithms/tracking/MPGDHitReconstruction.h"
#include "algorithms/tracking/MPGDHitReconstructionConfig.h"

using eicrecon::MPGDHitReconstruction;
using eicrecon::MPGDHitReconstructionConfig;

// Helper: build a CellID from field values using the IDDescriptor encoder.
// system=3 matches the mock geometry's addPhysVolID("system", 3).
static dd4hep::DDSegmentation::CellID makeCellID(const dd4hep::IDDescriptor& desc, int system,
                                                 int layer, int module, int strip, int x, int y) {
  auto encoder                       = desc.decoder();
  dd4hep::DDSegmentation::CellID cid = 0;
  encoder->set(cid, "system", system);
  encoder->set(cid, "layer", layer);
  encoder->set(cid, "module", module);
  encoder->set(cid, "strip", strip);
  encoder->set(cid, "x", x);
  encoder->set(cid, "y", y);
  return cid;
}

static dd4hep::IDDescriptor getMPGDIdDesc() {
  return algorithms::GeoSvc::instance().detector()->readout("MockMPGDHits").idSpec();
}

// ===========================================================================
// init() tests
// ===========================================================================

TEST_CASE("MPGDHitReconstruction: init succeeds with valid readout", "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_init_valid");
  MPGDHitReconstructionConfig cfg;
  cfg.readout = "MockMPGDHits";
  algo.applyConfig(cfg);
  REQUIRE_NOTHROW(algo.init());
}

TEST_CASE("MPGDHitReconstruction: init throws for missing strip field", "[MPGDHitReconstruction]") {
  // MockCalorimeterHits: "system:8,layer:8,x:8,y:8" — no "strip" field.
  MPGDHitReconstruction algo("test_init_nostrip");
  MPGDHitReconstructionConfig cfg;
  cfg.readout = "MockCalorimeterHits";
  algo.applyConfig(cfg);
  REQUIRE_THROWS(algo.init());
}

TEST_CASE("MPGDHitReconstruction: init throws for nonexistent readout", "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_init_noreadout");
  MPGDHitReconstructionConfig cfg;
  cfg.readout = "NoSuchReadout";
  algo.applyConfig(cfg);
  REQUIRE_THROWS(algo.init());
}

// ===========================================================================
// process() tests
// ===========================================================================

TEST_CASE("MPGDHitReconstruction: empty input produces empty output", "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_empty");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  algo.process({&raw_hits}, {&rec_hits});
  REQUIRE(rec_hits.size() == 0);
}

TEST_CASE("MPGDHitReconstruction: single p-strip hit produces one cluster",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_single_p");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  auto cellID = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  raw_hits.create(cellID, /*charge=*/500, /*timeStamp=*/100000);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  // Single-hit cluster: charge is the input charge (converted to GeV)
  CHECK(rec_hits[0].getEdep() > 0);
  // CellID of the reconstructed hit should match the max-charge hit
  CHECK(rec_hits[0].getCellID() == cellID);
  // Raw hit association
  CHECK(rec_hits[0].getRawHit().getCellID() == cellID);
}

TEST_CASE("MPGDHitReconstruction: single n-strip hit produces one cluster",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_single_n");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int nStrip = 2;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  auto cellID = makeCellID(id_desc, 3, 0, 0, nStrip, 0, 15);
  raw_hits.create(cellID, /*charge=*/700, /*timeStamp=*/200000);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  CHECK(rec_hits[0].getCellID() == cellID);
  CHECK(rec_hits[0].getRawHit().getCellID() == cellID);
}

TEST_CASE("MPGDHitReconstruction: two adjacent p-strip hits cluster together",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_adj_p");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // Two adjacent p-strip hits at x=10 and x=11 (same system/layer/module/strip)
  auto cid0 = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  auto cid1 = makeCellID(id_desc, 3, 0, 0, pStrip, 11, 0);
  raw_hits.create(cid0, /*charge=*/600, /*timeStamp=*/100000);
  raw_hits.create(cid1, /*charge=*/400, /*timeStamp=*/110000);

  algo.process({&raw_hits}, {&rec_hits});

  // Adjacent hits in the same subvolume and strip type → 1 cluster
  REQUIRE(rec_hits.size() == 1);
  // Total charge = 600 + 400 = 1000 (converted to GeV: 1000 / 1e6)
  CHECK(rec_hits[0].getEdep() > 0);
  // CellID from the max-charge hit
  CHECK(rec_hits[0].getCellID() == cid0);
  // Timing from the max-charge hit (600 > 400)
  CHECK(rec_hits[0].getTime() > 0);
}

TEST_CASE("MPGDHitReconstruction: two adjacent n-strip hits cluster together",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_adj_n");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int nStrip = 2;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  auto cid0 = makeCellID(id_desc, 3, 0, 0, nStrip, 0, 20);
  auto cid1 = makeCellID(id_desc, 3, 0, 0, nStrip, 0, 21);
  raw_hits.create(cid0, /*charge=*/300, /*timeStamp=*/100000);
  raw_hits.create(cid1, /*charge=*/500, /*timeStamp=*/150000);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  // CellID from the max-charge hit (500 > 300)
  CHECK(rec_hits[0].getCellID() == cid1);
}

TEST_CASE("MPGDHitReconstruction: separated hits produce separate clusters",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_separated");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // Two p-strip hits at x=5 and x=50 — far apart, not adjacent
  auto cid0 = makeCellID(id_desc, 3, 0, 0, pStrip, 5, 0);
  auto cid1 = makeCellID(id_desc, 3, 0, 0, pStrip, 50, 0);
  raw_hits.create(cid0, /*charge=*/500, /*timeStamp=*/100000);
  raw_hits.create(cid1, /*charge=*/500, /*timeStamp=*/100000);

  algo.process({&raw_hits}, {&rec_hits});

  // Non-adjacent → 2 separate clusters
  REQUIRE(rec_hits.size() == 2);
}

TEST_CASE("MPGDHitReconstruction: p-strip and n-strip hits produce separate clusters",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_pn_sep");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;
  const int nStrip = 2;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // One p-strip and one n-strip hit — different strip types → separate clusters
  auto cidP = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  auto cidN = makeCellID(id_desc, 3, 0, 0, nStrip, 0, 10);
  raw_hits.create(cidP, /*charge=*/500, /*timeStamp=*/100000);
  raw_hits.create(cidN, /*charge=*/500, /*timeStamp=*/100000);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 2);
}

TEST_CASE("MPGDHitReconstruction: three-hit cluster sums charge correctly",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_3hit");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // Three adjacent p-strip hits at x=10,11,12
  auto cid0 = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  auto cid1 = makeCellID(id_desc, 3, 0, 0, pStrip, 11, 0);
  auto cid2 = makeCellID(id_desc, 3, 0, 0, pStrip, 12, 0);
  raw_hits.create(cid0, /*charge=*/200, /*timeStamp=*/100000);
  raw_hits.create(cid1, /*charge=*/800, /*timeStamp=*/110000);
  raw_hits.create(cid2, /*charge=*/300, /*timeStamp=*/120000);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  // Charge is (200+800+300)/1e6 GeV = 0.0013
  float expected_edep = (200.0f + 800.0f + 300.0f) / 1.0e6f;
  CHECK(rec_hits[0].getEdep() == Catch::Approx(expected_edep).epsilon(0.01));
  // CellID from max-charge hit (800 at cid1)
  CHECK(rec_hits[0].getCellID() == cid1);
}

TEST_CASE("MPGDHitReconstruction: cluster timing from max-charge hit", "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_timing");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  auto cid0 = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  auto cid1 = makeCellID(id_desc, 3, 0, 0, pStrip, 11, 0);
  // Hit 1 has higher charge → its timestamp determines cluster time
  raw_hits.create(cid0, /*charge=*/300, /*timeStamp=*/100000);
  raw_hits.create(cid1, /*charge=*/700, /*timeStamp=*/200000);

  algo.process({&raw_hits}, {&rec_hits});

  REQUIRE(rec_hits.size() == 1);
  // Time = max-charge timestamp / 1000.0 (conversion to ns)
  float expected_time = 200000.0f / 1000.0f;
  CHECK(rec_hits[0].getTime() == Catch::Approx(expected_time).epsilon(0.01));
}

TEST_CASE("MPGDHitReconstruction: out-of-order input is sorted and clustered",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_sort");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // Insert in reverse coordinate order: x=11 first, x=10 second
  auto cid1 = makeCellID(id_desc, 3, 0, 0, pStrip, 11, 0);
  auto cid0 = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  raw_hits.create(cid1, /*charge=*/400, /*timeStamp=*/110000);
  raw_hits.create(cid0, /*charge=*/600, /*timeStamp=*/100000);

  algo.process({&raw_hits}, {&rec_hits});

  // Still clusters together (algorithm sorts internally)
  REQUIRE(rec_hits.size() == 1);
  // CellID from max-charge hit (600 at cid0)
  CHECK(rec_hits[0].getCellID() == cid0);
}

TEST_CASE("MPGDHitReconstruction: mixed p/n strips with adjacency", "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_mixed");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;
  const int nStrip = 2;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // Two adjacent p-strips + one n-strip
  auto cidP0 = makeCellID(id_desc, 3, 0, 0, pStrip, 10, 0);
  auto cidP1 = makeCellID(id_desc, 3, 0, 0, pStrip, 11, 0);
  auto cidN0 = makeCellID(id_desc, 3, 0, 0, nStrip, 0, 10);
  raw_hits.create(cidP0, /*charge=*/500, /*timeStamp=*/100000);
  raw_hits.create(cidP1, /*charge=*/500, /*timeStamp=*/100000);
  raw_hits.create(cidN0, /*charge=*/500, /*timeStamp=*/100000);

  algo.process({&raw_hits}, {&rec_hits});

  // p-strip cluster (2 hits) + n-strip cluster (1 hit) = 2 clusters
  REQUIRE(rec_hits.size() == 2);
}

TEST_CASE("MPGDHitReconstruction: different modules produce separate clusters",
          "[MPGDHitReconstruction]") {
  MPGDHitReconstruction algo("test_modules");
  MPGDHitReconstructionConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.timeResolution = 10.0;
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4eic::RawTrackerHitCollection raw_hits;
  edm4eic::TrackerHitCollection rec_hits;

  // Same strip type, same coordinates, but different modules
  auto cid0 = makeCellID(id_desc, 3, 0, /*module=*/0, pStrip, 10, 0);
  auto cid1 = makeCellID(id_desc, 3, 0, /*module=*/1, pStrip, 10, 0);
  raw_hits.create(cid0, /*charge=*/500, /*timeStamp=*/100000);
  raw_hits.create(cid1, /*charge=*/500, /*timeStamp=*/100000);

  algo.process({&raw_hits}, {&rec_hits});

  // Different modules → different subvolumes → 2 separate clusters
  REQUIRE(rec_hits.size() == 2);
}
