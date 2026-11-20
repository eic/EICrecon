// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <nopayloadclient/nopayloadclient.hpp>
#include <cmath>
#include <map>

#include "services/nopayloaddb/NoPayloadDb_service.h"

// Requires a locally reachable NoPayloadDB instance, configured via the
// NOPAYLOADCLIENT_CONF environment variable (see .github/workflows for an
// example that stands up postgres + nopayloaddb as service containers).
// Skipped rather than failed when that isn't set, so a plain local `ctest`
// run doesn't break in the absence of that database.
TEST_CASE("NoPayloadDb_service connects to the local test database", "[NoPayloadDb_service]") {
  if (std::getenv("NOPAYLOADCLIENT_CONF") == nullptr) {
    SKIP("NOPAYLOADCLIENT_CONF is not set; no local NoPayloadDB configured");
  }

  NoPayloadDb_service service(nullptr);

  nlohmann::json response = service.client().checkConnection();
  REQUIRE(response["code"] == 0);
}
