// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <nopayloadclient/nopayloadclient.hpp>
#include <cmath>
#include <map>

#include "services/nopayloaddb/NoPayloadDb_service.h"

// Requires a locally reachable NoPayloadDB instance, configured via the
// NOPAYLOADCLIENT_CONF environment variable (see .github/workflows for an
// example that stands up postgres + nopayloaddb as service containers).
TEST_CASE("NoPayloadDb_service connects to the local test database", "[NoPayloadDb_service]") {
  NoPayloadDb_service service(nullptr);

  nlohmann::json response = service.client().checkConnection();
  REQUIRE(response["code"] == 0);
}
