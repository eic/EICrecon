// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/EDM4hepVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoParticleLinkCollection.h>
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
#include <edm4hep/Vector2i.h>
#endif
#include <edm4hep/Vector3d.h>
#include <cmath>
#include <memory>

#include "algorithms/reco/ClustersToParticles.h"
#include "algorithms/reco/ClustersToParticlesConfig.h"

using eicrecon::ClustersToParticles;
using eicrecon::ClustersToParticlesConfig;

TEST_CASE("the ClustersToParticles algorithm runs", "[ClustersToParticles]") {
  const float EPSILON = 1e-5;

  ClustersToParticles algo("ClustersToParticles");
  algo.level(algorithms::LogLevel::kDebug);

  ClustersToParticlesConfig cfg;
  algo.applyConfig(cfg);
  algo.init();

  // Create input: two clusters at different positions with different energies,
  // plus one MC association on the first cluster
  edm4eic::ClusterCollection clusters;

  auto cluster1 = clusters.create();
  cluster1.setEnergy(5.0);
  cluster1.setPosition({0.0f, 0.0f, 100.0f});

  auto cluster2 = clusters.create();
  cluster2.setEnergy(10.0);
  // Cluster at 45 degrees in x-z plane
  cluster2.setPosition({1.0f, 0.0f, 1.0f});

  // Create an MC particle and associate it with cluster1
  edm4hep::MCParticleCollection mcparts;
  auto mcpart = mcparts.create(22,                  // std::int32_t PDG
                               1,                   // std::int32_t generatorStatus
                               0,                   // std::int32_t simulatorStatus
                               0.,                  // float charge
                               0.,                  // float time
                               0.,                  // double mass
                               edm4hep::Vector3d(), // edm4hep::Vector3d vertex
                               edm4hep::Vector3d(), // edm4hep::Vector3d endpoint
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 1)
                               edm4hep::Vector3f(), // edm4hep::Vector3f momentum
                               edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
#else
                               edm4hep::Vector3d(), // edm4hep::Vector3d momentum
                               edm4hep::Vector3d(), // edm4hep::Vector3d momentumAtEndpoint
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 3)
                               edm4hep::Vector3f() // edm4hep::Vector3f spin
#else
                               9 // int32_t helicity (9 if unset)
#endif
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
                               ,
                               edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
#endif
  );

  edm4eic::MCRecoClusterParticleAssociationCollection cluster_assocs;
  auto cluster_assoc = cluster_assocs.create();
  cluster_assoc.setRec(cluster1);
  cluster_assoc.setSim(mcpart);
  cluster_assoc.setWeight(0.9f);

  // Run algorithm
  auto parts       = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  auto part_assocs = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto part_links = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
  algo.process({&clusters, &cluster_assocs}, {parts.get(), part_links.get(), part_assocs.get()});
#else
  algo.process({&clusters, &cluster_assocs}, {parts.get(), part_assocs.get()});
#endif

  // Two clusters in, two particles out
  REQUIRE(parts->size() == 2);

  // --- First particle: from cluster1 at (0,0,100) with E=5 ---
  auto part1 = (*parts)[0];
  REQUIRE(part1.getPDG() == 22);
  REQUIRE_THAT(part1.getMass(), Catch::Matchers::WithinAbs(0.0, EPSILON));
  REQUIRE_THAT(part1.getCharge(), Catch::Matchers::WithinAbs(0.0, EPSILON));
  REQUIRE_THAT(part1.getEnergy(), Catch::Matchers::WithinAbs(5.0, EPSILON));
  // For massless particle |p| = E, directed along z
  REQUIRE_THAT(part1.getMomentum().x, Catch::Matchers::WithinAbs(0.0, EPSILON));
  REQUIRE_THAT(part1.getMomentum().y, Catch::Matchers::WithinAbs(0.0, EPSILON));
  REQUIRE_THAT(part1.getMomentum().z, Catch::Matchers::WithinAbs(5.0, EPSILON));

  // --- Second particle: from cluster2 at (1,0,1) with E=10 ---
  auto part2 = (*parts)[1];
  REQUIRE_THAT(part2.getEnergy(), Catch::Matchers::WithinAbs(10.0, EPSILON));
  // Position (1,0,1) -> unit vector (1/sqrt2, 0, 1/sqrt2), |p| = E = 10
  double expected_comp = 10.0 / std::sqrt(2.0);
  REQUIRE_THAT(part2.getMomentum().x, Catch::Matchers::WithinAbs(expected_comp, 1e-4));
  REQUIRE_THAT(part2.getMomentum().y, Catch::Matchers::WithinAbs(0.0, EPSILON));
  REQUIRE_THAT(part2.getMomentum().z, Catch::Matchers::WithinAbs(expected_comp, 1e-4));

  // --- Association: cluster1's MC association is propagated ---
  REQUIRE(part_assocs->size() == 1);
  auto assoc = (*part_assocs)[0];
  REQUIRE(assoc.getRec() == part1);
  REQUIRE(assoc.getSim() == mcpart);
  REQUIRE_THAT(assoc.getWeight(), Catch::Matchers::WithinAbs(0.9, EPSILON));

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  REQUIRE(part_links->size() == 1);
  auto link = (*part_links)[0];
  REQUIRE(link.getFrom() == part1);
  REQUIRE(link.getTo() == mcpart);
  REQUIRE_THAT(link.getWeight(), Catch::Matchers::WithinAbs(0.9, EPSILON));
#endif
}
