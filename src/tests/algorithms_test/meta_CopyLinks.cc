// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/MCRecoParticleLinkCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <deque>
#include <memory>

#include "algorithms/meta/CopyLinks.h"

TEST_CASE("the CopyLinks algorithm runs", "[CopyLinks]") {

  eicrecon::CopyLinks<edm4eic::ReconstructedParticle, edm4eic::MCRecoParticleLink> algo_copy_subset(
      "CopyLinksSubset");
  algo_copy_subset.level(algorithms::LogLevel::kDebug);
  algo_copy_subset.init();

  auto empty_rec_part_sub_coll    = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  auto empty_rec_part_coll        = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  auto empty_mc_rec_link_sub_coll = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
  auto empty_mc_rec_link_coll     = std::make_unique<edm4eic::MCRecoParticleLinkCollection>();
  algo_copy_subset.process(
      {empty_rec_part_sub_coll.get(), empty_rec_part_coll.get(), empty_mc_rec_link_sub_coll.get()},
      {empty_mc_rec_link_coll.get()});
  REQUIRE(empty_mc_rec_link_coll->size() == 0);
}
