// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <DD4hep/BitFieldCoder.h>
#include <DD4hep/IDDescriptor.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#endif

#include "algorithms/digi/MPGDTrackerDigi.h"
#include "algorithms/digi/MPGDTrackerDigiConfig.h"

using eicrecon::MPGDTrackerDigi;
using eicrecon::MPGDTrackerDigiConfig;

// Helper: build a CellID from field values using the IDDescriptor encoder.
// system=3 matches the mock geometry's addPhysVolID("system", 3).
// strip=1 → p-strip, strip=2 → n-strip
// sensor=0 (default, not used for non-CyMBaL detectors)
static dd4hep::DDSegmentation::CellID makeCellID(const dd4hep::IDDescriptor& desc, int system,
                                                 int layer, int module, int sensor, int strip,
                                                 int x, int y) {
  auto encoder                       = desc.decoder();
  dd4hep::DDSegmentation::CellID cid = 0;
  encoder->set(cid, "system", system);
  encoder->set(cid, "layer", layer);
  encoder->set(cid, "module", module);
  encoder->set(cid, "sensor", sensor);
  encoder->set(cid, "strip", strip);
  encoder->set(cid, "x", x);
  encoder->set(cid, "y", y);
  return cid;
}

static dd4hep::IDDescriptor getMPGDIdDesc() {
  return algorithms::GeoSvc::instance().detector()->readout("MockMPGDHits").idSpec();
}

// Helper: create a default MPGDTrackerDigiConfig for the mock geometry.
// The mock geometry uses 1 mm pitch CartesianGridXY segmentation.
// stripResolutions must be small relative to pitch (restriction I in the algorithm).
static MPGDTrackerDigiConfig makeDefaultConfig() {
  MPGDTrackerDigiConfig cfg;
  cfg.readout        = "MockMPGDHits";
  cfg.gain           = 10000;
  cfg.stripResolutions = {0.15, 0.15}; // 150 um in DD4hep units (mm)
  cfg.stripNumbers   = {1024, 1024};
  cfg.threshold      = 0.0;            // No threshold
  cfg.timeResolution = 8.0;
  return cfg;
}

// Helper: create a SimTrackerHit positioned in the mock MPGD geometry.
// The hit must have a valid CellID with strip field set (for VolumeManager lookup),
// a position in global coordinates consistent with the mock geometry placement,
// momentum for traversal direction, and nonzero pathLength.
// The mock sensor is a Box at z ~ ±0.025 inside module at z = imod*0.5.
// Module is inside envelope placed at world origin with system=3.
static void createSimHit(edm4hep::SimTrackerHitCollection& sim_hits,
                         edm4hep::MCParticleCollection& mc_particles,
                         dd4hep::DDSegmentation::CellID cellID,
                         double globalX, double globalY, double globalZ,
                         double momX, double momY, double momZ,
                         double eDep, double time, double pathLength) {
  auto particle = mc_particles.create();
  particle.setPDG(11); // electron
  particle.setMass(0.000511f);
  particle.setCharge(-1.0f);
  particle.setGeneratorStatus(1);

  auto hit = sim_hits.create();
  hit.setCellID(cellID);
  // SimTrackerHit positions are in EDM4hep units (mm)
  hit.setPosition({globalX, globalY, globalZ});
  hit.setMomentum({static_cast<float>(momX), static_cast<float>(momY), static_cast<float>(momZ)});
  hit.setEDep(eDep);
  hit.setTime(time);
  hit.setPathLength(pathLength);
  hit.setParticle(particle);
}

TEST_CASE("MPGDTrackerDigi: empty input produces empty output", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_empty");
  auto cfg = makeDefaultConfig();
  algo.applyConfig(cfg);
  algo.init();

  edm4hep::EventHeaderCollection headers;
  headers.create(0, 0); // eventNumber, runNumber
  edm4hep::SimTrackerHitCollection sim_hits;

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  REQUIRE(raw_hits.size() == 0);
  REQUIRE(associations.size() == 0);
}

TEST_CASE("MPGDTrackerDigi: single hit in p-strip sensor produces raw hits", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_single_p");
  auto cfg = makeDefaultConfig();
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4hep::EventHeaderCollection headers;
  headers.create(1, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  // CellID for p-strip (strip=1), module 0, layer 0, system 3
  auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);

  // Position: center of p-strip sensor at z = -0.025 (inside module 0 at z = 0)
  // Momentum along z (perpendicular to sensor) for simple traversal
  double eDep = 1.0e-6; // 1 keV in GeV (EDM4hep energy unit)
  createSimHit(sim_hits, mc_particles, cellID,
               0.0, 0.0, -0.025,   // position (mm)
               0.0, 0.0, 1.0,      // momentum (GeV)
               eDep, 10.0,          // eDep (GeV), time (ns)
               0.05);               // pathLength (mm) = sensor thickness

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  // A single sim hit should produce raw hits for both p and n strips (2-hit clusters each)
  REQUIRE(raw_hits.size() > 0);
  // Each raw hit should have a nonzero charge
  for (size_t i = 0; i < raw_hits.size(); i++) {
    CHECK(raw_hits[i].getCharge() > 0);
  }
  // Associations should link raw hits back to the sim hit
  CHECK(associations.size() > 0);
}

TEST_CASE("MPGDTrackerDigi: single hit in n-strip sensor produces raw hits", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_single_n");
  auto cfg = makeDefaultConfig();
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int nStrip = 2;

  edm4hep::EventHeaderCollection headers;
  headers.create(2, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  // CellID for n-strip (strip=2), module 0
  auto cellID = makeCellID(id_desc, 3, 0, 0, 0, nStrip, 0, 0);

  // Position: center of n-strip sensor at z = +0.025 (inside module 0 at z = 0)
  createSimHit(sim_hits, mc_particles, cellID,
               0.0, 0.0, 0.025,
               0.0, 0.0, 1.0,
               1.0e-6, 10.0, 0.05);

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  REQUIRE(raw_hits.size() > 0);
  for (size_t i = 0; i < raw_hits.size(); i++) {
    CHECK(raw_hits[i].getCharge() > 0);
  }
  CHECK(associations.size() > 0);
}

TEST_CASE("MPGDTrackerDigi: hit below threshold produces no output", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_threshold");
  auto cfg       = makeDefaultConfig();
  cfg.threshold  = 1.0e-3; // 1 MeV threshold (high, in DD4hep units = GeV)
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4hep::EventHeaderCollection headers;
  headers.create(3, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);

  // Very low energy deposit: 0.1 keV = 1e-7 GeV, well below 1 MeV threshold
  createSimHit(sim_hits, mc_particles, cellID,
               0.0, 0.0, -0.025,
               0.0, 0.0, 1.0,
               1.0e-7, 10.0, 0.05);

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  // Below threshold → no raw hits produced
  REQUIRE(raw_hits.size() == 0);
}

TEST_CASE("MPGDTrackerDigi: charge scales with energy deposit", "[MPGDTrackerDigi]") {
  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  // Run with low energy deposit
  int totalChargeLow = 0;
  {
    MPGDTrackerDigi algo("test_digi_charge_low");
    auto cfg = makeDefaultConfig();
    algo.applyConfig(cfg);
    algo.init();

    edm4hep::EventHeaderCollection headers;
    headers.create(4, 0);
    edm4hep::SimTrackerHitCollection sim_hits;
    edm4hep::MCParticleCollection mc_particles;

    auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
    createSimHit(sim_hits, mc_particles, cellID,
                 0.0, 0.0, -0.025,
                 0.0, 0.0, 1.0,
                 1.0e-6, 10.0, 0.05); // 1 keV

    edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
    edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
    algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

    for (size_t i = 0; i < raw_hits.size(); i++) {
      totalChargeLow += raw_hits[i].getCharge();
    }
  }

  // Run with high energy deposit (10x)
  int totalChargeHigh = 0;
  {
    MPGDTrackerDigi algo("test_digi_charge_high");
    auto cfg = makeDefaultConfig();
    algo.applyConfig(cfg);
    algo.init();

    edm4hep::EventHeaderCollection headers;
    headers.create(5, 0);
    edm4hep::SimTrackerHitCollection sim_hits;
    edm4hep::MCParticleCollection mc_particles;

    auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
    createSimHit(sim_hits, mc_particles, cellID,
                 0.0, 0.0, -0.025,
                 0.0, 0.0, 1.0,
                 10.0e-6, 10.0, 0.05); // 10 keV

    edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
    edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
    algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

    for (size_t i = 0; i < raw_hits.size(); i++) {
      totalChargeHigh += raw_hits[i].getCharge();
    }
  }

  // Higher energy deposit should produce proportionally higher total charge
  REQUIRE(totalChargeLow > 0);
  REQUIRE(totalChargeHigh > 0);
  CHECK(totalChargeHigh > totalChargeLow);
}

TEST_CASE("MPGDTrackerDigi: different modules produce independent raw hits", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_modules");
  auto cfg = makeDefaultConfig();
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4hep::EventHeaderCollection headers;
  headers.create(6, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  // Hit in module 0 (z = 0, sensor p-strip at z = -0.025)
  auto cellID0 = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
  createSimHit(sim_hits, mc_particles, cellID0,
               0.0, 0.0, -0.025,
               0.0, 0.0, 1.0,
               1.0e-6, 10.0, 0.05);

  // Hit in module 1 (z = 0.5, sensor p-strip at z = 0.5 - 0.025 = 0.475)
  auto cellID1 = makeCellID(id_desc, 3, 0, 1, 0, pStrip, 0, 0);
  createSimHit(sim_hits, mc_particles, cellID1,
               0.0, 0.0, 0.475,
               0.0, 0.0, 1.0,
               1.0e-6, 20.0, 0.05);

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  // Two independent sim hits should produce raw hits from each
  // (each produces hits for both p and n strips)
  REQUIRE(raw_hits.size() >= 4);
  // Each sim hit creates at least 2 associations (one per strip direction)
  CHECK(associations.size() >= 2);
}

TEST_CASE("MPGDTrackerDigi: gain parameter affects charge", "[MPGDTrackerDigi]") {
  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  int totalChargeDefaultGain = 0;
  {
    MPGDTrackerDigi algo("test_digi_gain_default");
    auto cfg = makeDefaultConfig();
    cfg.gain = 10000;
    algo.applyConfig(cfg);
    algo.init();

    edm4hep::EventHeaderCollection headers;
    headers.create(7, 0);
    edm4hep::SimTrackerHitCollection sim_hits;
    edm4hep::MCParticleCollection mc_particles;

    auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
    createSimHit(sim_hits, mc_particles, cellID,
                 0.0, 0.0, -0.025,
                 0.0, 0.0, 1.0,
                 1.0e-6, 10.0, 0.05);

    edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
    edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
    algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

    for (size_t i = 0; i < raw_hits.size(); i++) {
      totalChargeDefaultGain += raw_hits[i].getCharge();
    }
  }

  int totalChargeDoubleGain = 0;
  {
    MPGDTrackerDigi algo("test_digi_gain_double");
    auto cfg = makeDefaultConfig();
    cfg.gain = 20000; // Double the gain
    algo.applyConfig(cfg);
    algo.init();

    edm4hep::EventHeaderCollection headers;
    headers.create(8, 0);
    edm4hep::SimTrackerHitCollection sim_hits;
    edm4hep::MCParticleCollection mc_particles;

    auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
    createSimHit(sim_hits, mc_particles, cellID,
                 0.0, 0.0, -0.025,
                 0.0, 0.0, 1.0,
                 1.0e-6, 10.0, 0.05);

    edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
    edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
    algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

    for (size_t i = 0; i < raw_hits.size(); i++) {
      totalChargeDoubleGain += raw_hits[i].getCharge();
    }
  }

  // Double gain → double total charge
  REQUIRE(totalChargeDefaultGain > 0);
  REQUIRE(totalChargeDoubleGain > 0);
  // Allow some tolerance for rounding (charge is integer)
  double ratio = static_cast<double>(totalChargeDoubleGain) / totalChargeDefaultGain;
  CHECK(ratio == Catch::Approx(2.0).epsilon(0.01));
}

TEST_CASE("MPGDTrackerDigi: raw hit timestamps reflect sim hit time", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_timing");
  auto cfg       = makeDefaultConfig();
  cfg.timeResolution = 0.0; // No smearing for deterministic test
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4hep::EventHeaderCollection headers;
  headers.create(9, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  double simTime = 100.0; // ns
  auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
  createSimHit(sim_hits, mc_particles, cellID,
               0.0, 0.0, -0.025,
               0.0, 0.0, 1.0,
               1.0e-6, simTime, 0.05);

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  REQUIRE(raw_hits.size() > 0);
  // Timestamp is stored in ps: time_ns * 1e3
  // With zero time resolution, timestamps should be close to simTime * 1e3
  double expectedTimestamp = simTime * 1e3; // ps
  for (size_t i = 0; i < raw_hits.size(); i++) {
    double ts = static_cast<double>(raw_hits[i].getTimeStamp());
    // Allow some tolerance for the ToF correction applied during coalesce/extend
    CHECK(std::abs(ts - expectedTimestamp) < 1000.0); // within 1 ns
  }
}

TEST_CASE("MPGDTrackerDigi: associations link raw hits to sim hits", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_assoc");
  auto cfg = makeDefaultConfig();
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4hep::EventHeaderCollection headers;
  headers.create(10, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
  createSimHit(sim_hits, mc_particles, cellID,
               0.0, 0.0, -0.025,
               0.0, 0.0, 1.0,
               1.0e-6, 10.0, 0.05);

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  REQUIRE(raw_hits.size() > 0);
  REQUIRE(associations.size() > 0);

  // Each association should have weight 1.0 and reference valid sim hit
  for (size_t i = 0; i < associations.size(); i++) {
    CHECK(associations[i].getWeight() == Catch::Approx(1.0));
    // The associated sim hit should match our input
    CHECK(associations[i].getSimHit().getCellID() == cellID);
  }

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  // Links should also be created
  REQUIRE(links.size() > 0);
  for (size_t i = 0; i < links.size(); i++) {
    CHECK(links[i].getWeight() == Catch::Approx(1.0));
  }
#endif
}

TEST_CASE("MPGDTrackerDigi: produces both p-strip and n-strip raw hits", "[MPGDTrackerDigi]") {
  MPGDTrackerDigi algo("test_digi_pn_strips");
  auto cfg = makeDefaultConfig();
  algo.applyConfig(cfg);
  algo.init();

  auto id_desc     = getMPGDIdDesc();
  const int pStrip = 1;

  edm4hep::EventHeaderCollection headers;
  headers.create(11, 0);
  edm4hep::SimTrackerHitCollection sim_hits;
  edm4hep::MCParticleCollection mc_particles;

  // A hit in the p-strip sensor; the algorithm processes both p and n strips
  auto cellID = makeCellID(id_desc, 3, 0, 0, 0, pStrip, 0, 0);
  createSimHit(sim_hits, mc_particles, cellID,
               0.0, 0.0, -0.025,
               0.0, 0.0, 1.0,
               5.0e-6, 10.0, 0.05);

  edm4eic::RawTrackerHitCollection raw_hits;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  edm4eic::MCRecoTrackerHitLinkCollection links;
#endif
  edm4eic::MCRecoTrackerHitAssociationCollection associations;

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  algo.process({&headers, &sim_hits}, {&raw_hits, &links, &associations});
#else
  algo.process({&headers, &sim_hits}, {&raw_hits, &associations});
#endif

  // The algorithm produces 2-hit clusters for each of p and n strip directions
  // So we expect at least 2 raw hits (one or two per strip direction)
  REQUIRE(raw_hits.size() >= 2);

  // Check that raw hits span both strip types by examining cellIDs
  auto decoder = id_desc.decoder();
  bool hasPStrip = false, hasNStrip = false;
  for (size_t i = 0; i < raw_hits.size(); i++) {
    auto cid      = raw_hits[i].getCellID();
    int stripVal = decoder->get(cid, "strip");
    if (stripVal == 1) hasPStrip = true;
    if (stripVal == 2) hasNStrip = true;
  }
  CHECK(hasPStrip);
  CHECK(hasNStrip);
}
