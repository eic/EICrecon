// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <cmath>
#include <gsl/pointers>
#include <memory>

#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/TrackSeeding.h"
#include "algorithms/tracking/TrackSeedingConfig.h"

using eicrecon::TrackSeeding;
using eicrecon::TrackSeedingConfig;

TEST_CASE("TrackSeeding: three hits produce one seed with stable parameters", "[TrackSeeding]") {
  algorithms::ActsSvc::instance().init(std::make_shared<ActsGeometryProvider>());

  TrackSeeding algo("test_track_seeding");
  TrackSeedingConfig cfg;
  cfg.seedingMethod = TrackSeedingConfig::SeedingMethod::Auto;
  cfg.rMin              = 0.0f;
  cfg.rMax              = 1000.0f;
  cfg.zMin              = -1000.0f;
  cfg.zMax              = 1000.0f;
  cfg.minPt             = 0.01f;
  cfg.deltaRMin         = 0.0f;
  cfg.deltaRMax         = 1000.0f;
  cfg.deltaRMinBottomSP = 0.0f;
  cfg.deltaRMinTopSP    = 0.0f;
  cfg.deltaRMaxBottomSP = 1000.0f;
  cfg.deltaRMaxTopSP    = 1000.0f;
  algo.applyConfig(cfg);
  algo.init();

  edm4eic::TrackerHitCollection hits;
  hits.create(1, edm4hep::Vector3f(6.0f, 33.0f, 10.0f), edm4eic::CovDiag3f(), 0.0f, 0.0f, 1.0f,
              0.0f);
  hits.create(2, edm4hep::Vector3f(14.0f, 52.0f, 20.0f), edm4eic::CovDiag3f(), 0.0f, 0.0f, 1.0f,
              0.0f);
  hits.create(3, edm4hep::Vector3f(26.0f, 67.0f, 30.0f), edm4eic::CovDiag3f(), 0.0f, 0.0f, 1.0f,
              0.0f);

  edm4eic::TrackSeedCollection seeds;
  edm4eic::TrackParametersCollection params;
  algo.process({&hits}, {&seeds, &params});

  REQUIRE(seeds.size() == 1);
  REQUIRE(params.size() == 1);
  REQUIRE(seeds[0].getHits().size() == 3);

  const auto param = params[0];
  CHECK(std::isfinite(param.getPhi()));
  CHECK(std::isfinite(param.getTheta()));
  CHECK(std::isfinite(param.getQOverP()));
  CHECK(param.getTheta() == Catch::Approx(1.0666f).epsilon(0.1f));
  CHECK(param.getQOverP() != Catch::Approx(0.0f));
  CHECK(seeds[0].getParams().getQOverP() == Catch::Approx(param.getQOverP()));
}
