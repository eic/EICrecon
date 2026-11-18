// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#include <edm4eic/TrackClusterLinkCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "algorithms/calorimetry/CalorimeterEOverPCut.h"
#include "algorithms/calorimetry/CalorimeterEOverPCutConfig.h"

using Catch::Matchers::WithinAbs;
using eicrecon::CalorimeterEOverPCut;
using eicrecon::CalorimeterEOverPCutConfig;

namespace {

constexpr float EPSILON = 1e-6;

// MockCalorimeterHits readout: "system:8,layer:8,x:8,y:8"
std::uint64_t mockCellID(std::uint64_t layer) { return 1 | (layer << 8); }

struct EOverPOutputs {
  std::unique_ptr<edm4eic::ClusterCollection> clusters =
      std::make_unique<edm4eic::ClusterCollection>();
  std::unique_ptr<edm4eic::TrackClusterLinkCollection> track_links =
      std::make_unique<edm4eic::TrackClusterLinkCollection>();
  std::unique_ptr<edm4eic::MCRecoClusterParticleLinkCollection> mc_links =
      std::make_unique<edm4eic::MCRecoClusterParticleLinkCollection>();
  std::unique_ptr<edm4hep::ParticleIDCollection> pids =
      std::make_unique<edm4hep::ParticleIDCollection>();
};

// One cluster with three hits in layers 1, 2 and 3 (0.5 + 0.3 + 0.4 GeV)
struct EOverPInputs {
  edm4eic::CalorimeterHitCollection hits;
  edm4eic::ClusterCollection clusters;
  edm4eic::TrackCollection tracks;
  edm4eic::TrackClusterMatchCollection matches;
  edm4hep::MCParticleCollection mc_particles;
  edm4eic::MCRecoClusterParticleLinkCollection mc_links;

  EOverPInputs() {
    auto cluster = clusters.create();
    float energy = 0;
    const std::array<std::pair<std::uint64_t, float>, 3> layer_energies{
        {{1, 0.5F}, {2, 0.3F}, {3, 0.4F}}};
    for (const auto& [layer, hit_energy] : layer_energies) {
      auto hit = hits.create();
      hit.setCellID(mockCellID(layer));
      hit.setEnergy(hit_energy * edm4eic::unit::GeV);
      cluster.addToHits(hit);
      energy += hit.getEnergy();
    }
    cluster.setEnergy(energy);
  }

  void addMatch(float p, float weight) {
    auto track = tracks.create();
    track.setMomentum(edm4hep::Vector3f{0, 0, p * static_cast<float>(edm4eic::unit::GeV)});
    auto match = matches.create();
    match.setCluster(clusters[0]);
    match.setTrack(track);
    match.setWeight(weight);
  }

  void run(CalorimeterEOverPCut& algo, EOverPOutputs& out) const {
    algo.process({&clusters, &matches, &mc_links, &hits},
                 {out.clusters.get(), out.track_links.get(), out.mc_links.get(), out.pids.get()});
  }
};

void configure(CalorimeterEOverPCut& algo, const CalorimeterEOverPCutConfig& cfg) {
  algo.applyConfig(cfg);
  algo.init();
}

CalorimeterEOverPCutConfig fullEnergyConfig() {
  // maxLayer <= 0 uses the full cluster energy and needs no cellID decoding
  return {.eOverPCut = 0.74, .maxLayer = 0, .readout = "", .layerField = ""};
}

} // namespace

TEST_CASE("E/p cut selects the highest-weight track match", "[CalorimeterEOverPCut]") {
  CalorimeterEOverPCut algo("CalorimeterEOverPCut");
  configure(algo, fullEnergyConfig());

  EOverPInputs in;
  // E = 1.2 GeV: the 2 GeV track would fail the cut (E/p = 0.6), the 1 GeV track passes (1.2)
  in.addMatch(2.0, 0.3);
  in.addMatch(1.0, 0.7);

  EOverPOutputs out;
  in.run(algo, out);

  REQUIRE(out.clusters->size() == 1);
  REQUIRE(out.pids->size() == 1);
  const auto pid = (*out.pids)[0];
  CHECK(pid.getPDG() == 11);
  CHECK_THAT(pid.getLikelihood(), WithinAbs(1.0, EPSILON));
  REQUIRE(pid.parameters_size() == 1);
  CHECK_THAT(pid.getParameters(0), WithinAbs(1.2, EPSILON));
  REQUIRE((*out.clusters)[0].particleIDs_size() == 1);
  CHECK((*out.clusters)[0].getParticleIDs(0) == pid);

  // All track matches of a selected cluster are carried over as links
  REQUIRE(out.track_links->size() == 2);
  for (std::size_t i = 0; i < out.track_links->size(); ++i) {
    const auto link = (*out.track_links)[i];
    CHECK(link.getFrom() == (*out.clusters)[0]);
    CHECK(link.getTo() == in.matches[i].getTrack());
    CHECK_THAT(link.getWeight(), WithinAbs(in.matches[i].getWeight(), EPSILON));
  }
}

TEST_CASE("E/p cut rejects clusters below threshold or without a match", "[CalorimeterEOverPCut]") {
  CalorimeterEOverPCut algo("CalorimeterEOverPCut");
  configure(algo, fullEnergyConfig());

  SECTION("no track match") {
    EOverPInputs in;
    EOverPOutputs out;
    in.run(algo, out);
    CHECK(out.clusters->empty());
    CHECK(out.track_links->empty());
    CHECK(out.mc_links->empty());
    CHECK(out.pids->empty());
  }

  SECTION("E/p below threshold") {
    EOverPInputs in;
    in.addMatch(2.0, 1.0); // E/p = 0.6
    EOverPOutputs out;
    in.run(algo, out);
    CHECK(out.clusters->empty());
    CHECK(out.track_links->empty());
    CHECK(out.pids->empty());
  }
}

TEST_CASE("E/p cut sums hit energies up to maxLayer", "[CalorimeterEOverPCut]") {
  CalorimeterEOverPCutConfig cfg{
      .eOverPCut = 0.74, .maxLayer = 2, .readout = "MockCalorimeterHits", .layerField = "layer"};

  SECTION("layers 1-2 pass") {
    CalorimeterEOverPCut algo("CalorimeterEOverPCut");
    configure(algo, cfg);
    EOverPInputs in;
    in.addMatch(1.0, 1.0); // E(layer <= 2) = 0.8 GeV
    EOverPOutputs out;
    in.run(algo, out);
    REQUIRE(out.pids->size() == 1);
    CHECK_THAT((*out.pids)[0].getParameters(0), WithinAbs(0.8, EPSILON));
  }

  SECTION("layer 1 only fails") {
    cfg.maxLayer = 1;
    CalorimeterEOverPCut algo("CalorimeterEOverPCut");
    configure(algo, cfg);
    EOverPInputs in;
    in.addMatch(1.0, 1.0); // E(layer <= 1) = 0.5 GeV
    EOverPOutputs out;
    in.run(algo, out);
    CHECK(out.clusters->empty());
  }

  SECTION("unknown readout falls back to the full cluster energy") {
    cfg.readout = "NonexistentReadout";
    CalorimeterEOverPCut algo("CalorimeterEOverPCut");
    configure(algo, cfg);
    EOverPInputs in;
    in.addMatch(1.0, 1.0);
    EOverPOutputs out;
    in.run(algo, out);
    REQUIRE(out.pids->size() == 1);
    CHECK_THAT((*out.pids)[0].getParameters(0), WithinAbs(1.2, EPSILON));
  }
}

TEST_CASE("E/p cut propagates MC links to selected clusters", "[CalorimeterEOverPCut]") {
  CalorimeterEOverPCut algo("CalorimeterEOverPCut");
  configure(algo, fullEnergyConfig());

  EOverPInputs in;
  in.addMatch(1.0, 1.0);
  auto mc_particle = in.mc_particles.create();
  auto mc_link     = in.mc_links.create();
  mc_link.setFrom(in.clusters[0]);
  mc_link.setTo(mc_particle);
  mc_link.setWeight(0.6);

  EOverPOutputs out;
  in.run(algo, out);

  REQUIRE(out.clusters->size() == 1);
  REQUIRE(out.mc_links->size() == 1);
  const auto out_link = (*out.mc_links)[0];
  CHECK(out_link.getFrom() == (*out.clusters)[0]);
  CHECK(out_link.getTo() == mc_particle);
  CHECK_THAT(out_link.getWeight(), WithinAbs(0.6, EPSILON));
}
