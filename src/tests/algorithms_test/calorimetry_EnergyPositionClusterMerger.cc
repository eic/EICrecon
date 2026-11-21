// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Chun Yuen Tsang

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleLinkCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <podio/detail/LinkCollectionIterator.h>
#include <cmath>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <memory>
#include <optional>
#include <vector>

#include "algorithms/calorimetry/EnergyPositionClusterMerger.h"
#include "algorithms/calorimetry/EnergyPositionClusterMergerConfig.h"

namespace {

using edm4eic::unit::GeV;
using edm4eic::unit::mm;
using edm4eic::unit::ns;

//! Create a cluster at (eta, phi) with the given energy at radius r, with nhits hits attached
edm4eic::MutableCluster make_cluster(edm4eic::ClusterCollection& coll,
                                     edm4eic::CalorimeterHitCollection& hits, float energy,
                                     float eta, float phi, float time, std::size_t nhits,
                                     float r = 2000. * mm) {
  auto cluster = coll.create();
  cluster.setEnergy(energy);
  cluster.setEnergyError(0.1F * energy);
  cluster.setTime(time);
  cluster.setTimeError(0.5 * ns);
  cluster.setPosition(
      {r * std::cos(phi) / std::cosh(eta), r * std::sin(phi) / std::cosh(eta), r * std::tanh(eta)});
  // scale the covariance with energy so different clusters carry distinguishable errors
  const double s = energy / GeV;
  cluster.setPositionError(
      edm4eic::Cov3f{s * 1.0 * mm * mm, s * 2.0 * mm * mm, s * 3.0 * mm * mm, s * 0.1 * mm * mm});
  cluster.setNhits(nhits);
  for (std::size_t i = 0; i < nhits; ++i) {
    auto hit = hits.create();
    hit.setEnergy(energy / nhits);
    hit.setPosition(cluster.getPosition());
    cluster.addToHits(hit);
  }
  return cluster;
}

void add_assoc(edm4eic::MCRecoClusterParticleAssociationCollection& assocs,
               const edm4eic::Cluster& cluster, const edm4hep::MCParticle& particle) {
  auto assoc = assocs.create();
  assoc.setRec(cluster);
  assoc.setSim(particle);
  assoc.setWeight(1.0);
}

//! Find the merged cluster whose constituent clusters include the given cluster
std::optional<edm4eic::Cluster> find_merged_containing(const edm4eic::ClusterCollection& merged,
                                                       const edm4eic::Cluster& constituent) {
  for (const auto& cl : merged) {
    for (const auto& sub : cl.getClusters()) {
      if (sub == constituent) {
        return cl;
      }
    }
  }
  return std::nullopt;
}

//! Require that `out` is an unchanged copy of the energy cluster `ec`
void require_copy_of(const edm4eic::Cluster& out, const edm4eic::Cluster& ec) {
  REQUIRE(out.getEnergy() == ec.getEnergy());
  REQUIRE(out.getEnergyError() == ec.getEnergyError());
  REQUIRE(out.getTime() == ec.getTime());
  REQUIRE(out.getTimeError() == ec.getTimeError());
  REQUIRE(out.getNhits() == ec.getNhits());
  REQUIRE(out.getPosition().x == ec.getPosition().x);
  REQUIRE(out.getPosition().y == ec.getPosition().y);
  REQUIRE(out.getPosition().z == ec.getPosition().z);
  REQUIRE(out.getPositionError().xx == ec.getPositionError().xx);
  REQUIRE(out.getPositionError().yy == ec.getPositionError().yy);
  REQUIRE(out.getPositionError().zz == ec.getPositionError().zz);
  REQUIRE(out.getPositionError().xy == ec.getPositionError().xy);
  REQUIRE(out.clusters_size() == 1);
  REQUIRE(out.getClusters(0) == ec);
  REQUIRE(out.hits_size() == ec.hits_size());
  for (std::size_t i = 0; i < ec.hits_size(); ++i) {
    REQUIRE(out.getHits(i) == ec.getHits(i));
  }
  REQUIRE(out.subdetectorEnergies_size() == 1);
  REQUIRE(out.getSubdetectorEnergies(0) == ec.getEnergy());
}

//! Config with explicit tolerances so the geometry in the tests is meaningful.
//! The fallBackToECOnly flag is deliberately left at its default.
eicrecon::EnergyPositionClusterMergerConfig make_config() {
  eicrecon::EnergyPositionClusterMergerConfig cfg;
  cfg.energyRelTolerance = 0.5;
  cfg.phiTolerance       = 0.1;
  cfg.etaTolerance       = 0.2;
  return cfg;
}

//! Algorithm, input/output collections and MC particles shared by all sections
struct Fixture {
  eicrecon::EnergyPositionClusterMerger algo{"EnergyPositionClusterMerger"};

  std::unique_ptr<edm4eic::CalorimeterHitCollection> hits =
      std::make_unique<edm4eic::CalorimeterHitCollection>();
  std::unique_ptr<edm4hep::MCParticleCollection> particles =
      std::make_unique<edm4hep::MCParticleCollection>();
  std::unique_ptr<edm4eic::ClusterCollection> energy_clus =
      std::make_unique<edm4eic::ClusterCollection>();
  std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection> energy_asoc =
      std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();
  std::unique_ptr<edm4eic::ClusterCollection> pos_clus =
      std::make_unique<edm4eic::ClusterCollection>();
  std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection> pos_asoc =
      std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

  std::unique_ptr<edm4eic::ClusterCollection> merged_clus =
      std::make_unique<edm4eic::ClusterCollection>();
  std::unique_ptr<edm4eic::MCRecoClusterParticleLinkCollection> merged_links =
      std::make_unique<edm4eic::MCRecoClusterParticleLinkCollection>();
  std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection> merged_assoc =
      std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

  edm4hep::MutableMCParticle mc_a;
  edm4hep::MutableMCParticle mc_b;

  explicit Fixture(const eicrecon::EnergyPositionClusterMergerConfig& cfg)
      : mc_a(particles->create()), mc_b(particles->create()) {
    mc_a.setPDG(11);
    mc_b.setPDG(22);
    algo.applyConfig(cfg);
    algo.level(algorithms::LogLevel::kDebug);
    algo.init();
  }

  void run() {
    algo.process({energy_clus.get(), energy_asoc.get(), pos_clus.get(), pos_asoc.get()},
                 {merged_clus.get(), merged_links.get(), merged_assoc.get()});
  }

  void require_all_empty() const {
    REQUIRE(merged_clus->empty());
    REQUIRE(merged_links->empty());
    REQUIRE(merged_assoc->empty());
  }
};

//! Require that exactly one merged cluster exists, combining `ec` and `pc`, associated to mc_a
void require_matched_pair(const Fixture& fx, const edm4eic::Cluster& ec,
                          const edm4eic::Cluster& pc) {
  REQUIRE(fx.merged_clus->size() == 1);
  const auto out = (*fx.merged_clus)[0];
  REQUIRE(out.getEnergy() == ec.getEnergy());
  REQUIRE(out.getEnergyError() == ec.getEnergyError());
  REQUIRE(out.getTime() == pc.getTime());
  REQUIRE(out.getTimeError() == pc.getTimeError());
  REQUIRE(out.getPosition().x == pc.getPosition().x);
  REQUIRE(out.getPosition().y == pc.getPosition().y);
  REQUIRE(out.getPosition().z == pc.getPosition().z);
  REQUIRE(out.getNhits() == pc.getNhits() + ec.getNhits());
  REQUIRE(out.clusters_size() == 2);
  REQUIRE(out.getClusters(0) == pc);
  REQUIRE(out.getClusters(1) == ec);
  REQUIRE(out.getPositionError().xx == pc.getPositionError().xx);
  REQUIRE(out.getPositionError().xx != ec.getPositionError().xx);
  REQUIRE(out.hits_size() == 3);
  REQUIRE(out.getHits(0) == pc.getHits(0));
  REQUIRE(out.getHits(1) == ec.getHits(0));
  REQUIRE(out.getHits(2) == ec.getHits(1));
  REQUIRE(out.subdetectorEnergies_size() == 2);
  REQUIRE(out.getSubdetectorEnergies(0) == pc.getEnergy());
  REQUIRE(out.getSubdetectorEnergies(1) == ec.getEnergy());

  REQUIRE(fx.merged_links->size() == 1);
  REQUIRE((*fx.merged_links)[0].getFrom() == out);
  REQUIRE((*fx.merged_links)[0].getTo() == fx.mc_a);
  REQUIRE((*fx.merged_links)[0].getWeight() == 1.0F);
  REQUIRE(fx.merged_assoc->size() == 1);
  REQUIRE((*fx.merged_assoc)[0].getRec() == out);
  REQUIRE((*fx.merged_assoc)[0].getSim() == fx.mc_a);
  REQUIRE((*fx.merged_assoc)[0].getWeight() == 1.0F);
}

//! Require that `ec` and `pc` were merged into one cluster (position cluster first)
void require_merged_with(const Fixture& fx, const edm4eic::Cluster& ec,
                         const edm4eic::Cluster& pc) {
  const auto out = find_merged_containing(*fx.merged_clus, ec);
  REQUIRE(out.has_value());
  REQUIRE(out->clusters_size() == 2);
  REQUIRE(out->getClusters(0) == pc);
  REQUIRE(out->getEnergy() == ec.getEnergy());
  REQUIRE(out->getPosition().z == pc.getPosition().z);
  REQUIRE(out->getNhits() == ec.getNhits() + pc.getNhits());
}

//! Require that the output cluster containing `ec` has exactly one association and link, to `mc`
void require_single_assoc(const Fixture& fx, const edm4eic::Cluster& ec,
                          const edm4hep::MCParticle& mc) {
  const auto out = find_merged_containing(*fx.merged_clus, ec);
  REQUIRE(out.has_value());
  std::size_t n = 0;
  for (const auto& as : *fx.merged_assoc) {
    if (as.getRec() == *out) {
      REQUIRE(as.getSim() == mc);
      ++n;
    }
  }
  REQUIRE(n == 1);
  std::size_t nl = 0;
  for (const auto& link : *fx.merged_links) {
    if (link.getFrom() == *out) {
      REQUIRE(link.getTo() == mc);
      ++nl;
    }
  }
  REQUIRE(nl == 1);
}

//! Count the output clusters that contain `ec` as a constituent
std::size_t count_merged_containing(const edm4eic::ClusterCollection& merged,
                                    const edm4eic::Cluster& ec) {
  std::size_t n = 0;
  for (const auto& cl : merged) {
    for (const auto& sub : cl.getClusters()) {
      n += (sub == ec) ? 1 : 0;
    }
  }
  return n;
}

} // namespace

TEST_CASE("the EnergyPositionClusterMerger default config disables energy-only fallback",
          "[EnergyPositionClusterMerger]") {
  REQUIRE_FALSE(eicrecon::EnergyPositionClusterMergerConfig{}.fallBackToECOnly);
}

TEST_CASE("the EnergyPositionClusterMerger algorithm runs with energy-only fallback enabled",
          "[EnergyPositionClusterMerger]") {

  auto cfg             = make_config();
  cfg.fallBackToECOnly = true;
  Fixture fx(cfg);

  SECTION("matched energy and position clusters are combined") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    auto pc = make_cluster(*fx.pos_clus, *fx.hits, 0.9 * GeV, 0.52F, 0.51F, 3.0 * ns, 1);
    add_assoc(*fx.energy_asoc, ec, fx.mc_a);
    add_assoc(*fx.pos_asoc, pc, fx.mc_a);

    fx.run();

    require_matched_pair(fx, ec, pc);
  }

  SECTION("energy cluster with no position clusters is copied over") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    add_assoc(*fx.energy_asoc, ec, fx.mc_a);

    fx.run();

    REQUIRE(fx.merged_clus->size() == 1);
    const auto out = (*fx.merged_clus)[0];
    require_copy_of(out, ec);

    REQUIRE(fx.merged_links->size() == 1);
    REQUIRE((*fx.merged_links)[0].getFrom() == out);
    REQUIRE((*fx.merged_links)[0].getTo() == fx.mc_a);
    REQUIRE((*fx.merged_links)[0].getWeight() == 1.0F);
    REQUIRE(fx.merged_assoc->size() == 1);
    REQUIRE((*fx.merged_assoc)[0].getRec() == out);
    REQUIRE((*fx.merged_assoc)[0].getSim() == fx.mc_a);
    REQUIRE((*fx.merged_assoc)[0].getWeight() == 1.0F);
  }

  SECTION("energy cluster with only out-of-tolerance position clusters is copied over") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    // fails the eta cut (deta = 1.5 > 0.2)
    auto pc_eta = make_cluster(*fx.pos_clus, *fx.hits, 1.0 * GeV, 2.00F, 0.50F, 3.0 * ns, 1);
    // fails the phi cut (dphi = 1.0 > 0.1)
    auto pc_phi = make_cluster(*fx.pos_clus, *fx.hits, 1.0 * GeV, 0.50F, 1.50F, 3.0 * ns, 1);
    // fails the relative energy cut (|dE/E| = 2 > 0.5)
    auto pc_e = make_cluster(*fx.pos_clus, *fx.hits, 3.0 * GeV, 0.50F, 0.50F, 3.0 * ns, 1);
    add_assoc(*fx.energy_asoc, ec, fx.mc_a);
    add_assoc(*fx.pos_asoc, pc_eta, fx.mc_b);

    fx.run();

    // unmatched position clusters are not propagated; only the energy cluster copy remains
    REQUIRE(fx.merged_clus->size() == 1);
    const auto out = (*fx.merged_clus)[0];
    require_copy_of(out, ec);

    REQUIRE(fx.merged_assoc->size() == 1);
    REQUIRE((*fx.merged_assoc)[0].getRec() == out);
    REQUIRE((*fx.merged_assoc)[0].getSim() == fx.mc_a);
    REQUIRE(fx.merged_links->size() == 1);
    REQUIRE((*fx.merged_links)[0].getFrom() == out);
    REQUIRE((*fx.merged_links)[0].getTo() == fx.mc_a);
  }

  SECTION("unmatched position clusters are dropped") {
    auto pc = make_cluster(*fx.pos_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 3.0 * ns, 1);
    add_assoc(*fx.pos_asoc, pc, fx.mc_a);

    fx.run();

    // a position cluster with no energy cluster produces no output cluster of its own
    fx.require_all_empty();
  }

  SECTION("unmatched energy cluster without association produces no association") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);

    fx.run();

    REQUIRE(fx.merged_clus->size() == 1);
    require_copy_of((*fx.merged_clus)[0], ec);
    REQUIRE(fx.merged_links->empty());
    REQUIRE(fx.merged_assoc->empty());
  }

  SECTION("mixed matched and unmatched energy clusters") {
    auto ec0 = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    auto ec1 = make_cluster(*fx.energy_clus, *fx.hits, 2.0 * GeV, -1.00F, 2.00F, 6.0 * ns, 3);
    auto ec2 = make_cluster(*fx.energy_clus, *fx.hits, 4.0 * GeV, 1.50F, -1.00F, 7.0 * ns, 1);
    add_assoc(*fx.energy_asoc, ec0, fx.mc_a);
    add_assoc(*fx.energy_asoc, ec1, fx.mc_b);
    add_assoc(*fx.energy_asoc, ec2, fx.mc_a);

    auto pc0 = make_cluster(*fx.pos_clus, *fx.hits, 1.1 * GeV, 0.52F, 0.51F, 3.0 * ns, 1);
    auto pc2 = make_cluster(*fx.pos_clus, *fx.hits, 3.8 * GeV, 1.48F, -1.01F, 4.0 * ns, 2);
    add_assoc(*fx.pos_asoc, pc0, fx.mc_a);
    add_assoc(*fx.pos_asoc, pc2, fx.mc_a);

    fx.run();

    REQUIRE(fx.merged_clus->size() == 3);
    REQUIRE(fx.merged_assoc->size() == 3);
    REQUIRE(fx.merged_links->size() == 3);

    // each energy cluster appears in exactly one output cluster
    for (const auto& ec : {ec0, ec1, ec2}) {
      REQUIRE(count_merged_containing(*fx.merged_clus, ec) == 1);
    }

    require_merged_with(fx, ec0, pc0);
    require_merged_with(fx, ec2, pc2);

    const auto out1 = find_merged_containing(*fx.merged_clus, ec1);
    REQUIRE(out1.has_value());
    require_copy_of(*out1, ec1);

    // each output cluster has one association to the expected particle
    require_single_assoc(fx, ec0, fx.mc_a);
    require_single_assoc(fx, ec1, fx.mc_b);
    require_single_assoc(fx, ec2, fx.mc_a);
  }
}

TEST_CASE(
    "the EnergyPositionClusterMerger algorithm runs with energy-only fallback disabled (default)",
    "[EnergyPositionClusterMerger]") {

  // fallBackToECOnly is not set: the default value is what is exercised
  Fixture fx(make_config());

  SECTION("matched energy and position clusters are combined") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    auto pc = make_cluster(*fx.pos_clus, *fx.hits, 0.9 * GeV, 0.52F, 0.51F, 3.0 * ns, 1);
    add_assoc(*fx.energy_asoc, ec, fx.mc_a);
    add_assoc(*fx.pos_asoc, pc, fx.mc_a);

    fx.run();

    require_matched_pair(fx, ec, pc);
  }

  SECTION("energy cluster with no position clusters is not copied over") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    add_assoc(*fx.energy_asoc, ec, fx.mc_a);

    fx.run();

    fx.require_all_empty();
  }

  SECTION("energy cluster with only out-of-tolerance position clusters is not copied over") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    // fails the eta cut (deta = 1.5 > 0.2)
    auto pc_eta = make_cluster(*fx.pos_clus, *fx.hits, 1.0 * GeV, 2.00F, 0.50F, 3.0 * ns, 1);
    // fails the phi cut (dphi = 1.0 > 0.1)
    auto pc_phi = make_cluster(*fx.pos_clus, *fx.hits, 1.0 * GeV, 0.50F, 1.50F, 3.0 * ns, 1);
    // fails the relative energy cut (|dE/E| = 2 > 0.5)
    auto pc_e = make_cluster(*fx.pos_clus, *fx.hits, 3.0 * GeV, 0.50F, 0.50F, 3.0 * ns, 1);
    add_assoc(*fx.energy_asoc, ec, fx.mc_a);
    add_assoc(*fx.pos_asoc, pc_eta, fx.mc_b);

    fx.run();

    fx.require_all_empty();
  }

  SECTION("unmatched position clusters are dropped") {
    auto pc = make_cluster(*fx.pos_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 3.0 * ns, 1);
    add_assoc(*fx.pos_asoc, pc, fx.mc_a);

    fx.run();

    fx.require_all_empty();
  }

  SECTION("unmatched energy cluster without association is not copied over") {
    auto ec = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);

    fx.run();

    fx.require_all_empty();
  }

  SECTION("mixed matched and unmatched energy clusters drops the unmatched one") {
    auto ec0 = make_cluster(*fx.energy_clus, *fx.hits, 1.0 * GeV, 0.50F, 0.50F, 5.0 * ns, 2);
    auto ec1 = make_cluster(*fx.energy_clus, *fx.hits, 2.0 * GeV, -1.00F, 2.00F, 6.0 * ns, 3);
    auto ec2 = make_cluster(*fx.energy_clus, *fx.hits, 4.0 * GeV, 1.50F, -1.00F, 7.0 * ns, 1);
    add_assoc(*fx.energy_asoc, ec0, fx.mc_a);
    add_assoc(*fx.energy_asoc, ec1, fx.mc_b);
    add_assoc(*fx.energy_asoc, ec2, fx.mc_a);

    auto pc0 = make_cluster(*fx.pos_clus, *fx.hits, 1.1 * GeV, 0.52F, 0.51F, 3.0 * ns, 1);
    auto pc2 = make_cluster(*fx.pos_clus, *fx.hits, 3.8 * GeV, 1.48F, -1.01F, 4.0 * ns, 2);
    add_assoc(*fx.pos_asoc, pc0, fx.mc_a);
    add_assoc(*fx.pos_asoc, pc2, fx.mc_a);

    fx.run();

    REQUIRE(fx.merged_clus->size() == 2);
    REQUIRE(fx.merged_assoc->size() == 2);
    REQUIRE(fx.merged_links->size() == 2);

    // the unmatched energy cluster appears in no output cluster
    REQUIRE(count_merged_containing(*fx.merged_clus, ec1) == 0);
    REQUIRE(count_merged_containing(*fx.merged_clus, ec0) == 1);
    REQUIRE(count_merged_containing(*fx.merged_clus, ec2) == 1);

    require_merged_with(fx, ec0, pc0);
    require_merged_with(fx, ec2, pc2);

    require_single_assoc(fx, ec0, fx.mc_a);
    require_single_assoc(fx, ec2, fx.mc_a);
  }
}
