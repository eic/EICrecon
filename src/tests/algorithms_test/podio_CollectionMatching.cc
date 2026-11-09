// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>
#include <vector>

#include "services/io/podio/CollectionMatching.h"

using eicrecon::match_collections;
using Names = std::vector<std::string>;

TEST_CASE("match_collections: exact names, regexes and exclusions", "[podio]") {
  const std::set<std::string> collections{"EcalBarrelClusters", "EcalEndcapNClusters",
                                          "EcalEndcapPClusters", "EventHeader", "MCParticles"};

  SECTION("exact names") {
    auto m = match_collections(collections, {"EventHeader", "MCParticles"}, {});
    REQUIRE(m.matched == Names{"EventHeader", "MCParticles"});
    REQUIRE(m.unmatched_patterns.empty());
  }

  SECTION("a name that is not present is reported") {
    auto m = match_collections(collections, {"EventHeader", "NoSuchCollection"}, {});
    REQUIRE(m.matched == Names{"EventHeader"});
    REQUIRE(m.unmatched_patterns == Names{"NoSuchCollection"});
  }

  SECTION("a regex matching several collections") {
    auto m = match_collections(collections, {"Ecal.*Clusters"}, {});
    REQUIRE(m.matched == Names{"EcalBarrelClusters", "EcalEndcapNClusters", "EcalEndcapPClusters"});
    REQUIRE(m.unmatched_patterns.empty());
  }

  SECTION("a regex matching nothing is reported") {
    auto m = match_collections(collections, {"Hcal.*Clusters", "EventHeader"}, {});
    REQUIRE(m.matched == Names{"EventHeader"});
    REQUIRE(m.unmatched_patterns == Names{"Hcal.*Clusters"});
  }

  SECTION("a collection matched by two patterns uses both") {
    auto m = match_collections(collections, {"EcalEndcapPClusters", "Ecal.*Clusters"}, {});
    REQUIRE(m.matched == Names{"EcalBarrelClusters", "EcalEndcapNClusters", "EcalEndcapPClusters"});
    REQUIRE(m.unmatched_patterns.empty());
  }

  SECTION("a regex must match the whole name") {
    auto m = match_collections(collections, {"Ecal"}, {});
    REQUIRE(m.matched.empty());
    REQUIRE(m.unmatched_patterns == Names{"Ecal"});
  }

  SECTION("excluded collections are not written but still count as matched") {
    auto m = match_collections(collections, {"Ecal.*Clusters", "EventHeader"},
                               {"EcalBarrelClusters", "EventHeader"});
    REQUIRE(m.matched == Names{"EcalEndcapNClusters", "EcalEndcapPClusters"});
    REQUIRE(m.unmatched_patterns.empty());
  }
}
