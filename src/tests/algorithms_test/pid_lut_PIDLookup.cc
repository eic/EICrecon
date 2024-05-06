// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/Cov4f.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/Vector2i.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <spdlog/common.h>
#include <memory>

#include "algorithms/pid_lut/PIDLookup.h"
#include "algorithms/pid_lut/PIDLookupConfig.h"

using eicrecon::PIDLookup;
using eicrecon::PIDLookupConfig;

TEST_CASE( "particles acquire PID", "[PIDLookup]" ) {
  PIDLookup algo("test");

  PIDLookupConfig cfg {
    .filename="/dev/null",
    .system=0xFF,
    .pdg_values={11},
    .charge_values={1},
    .momentum_edges={0., 1., 2.},
    .polar_edges={0., M_PI},
    .azimuthal_binning={0., 2 * M_PI, 2 * M_PI}, // lower, upper, step
    .momentum_bin_centers_in_lut=true,
    .polar_bin_centers_in_lut=true,
    .use_radians=true,
  };

  SECTION( "single hit with couple contributions" ) {
    algo.level(algorithms::LogLevel(spdlog::level::trace));
    algo.applyConfig(cfg);
    algo.init();

    auto parts_in = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto assocs_in = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto mcparts = std::make_unique<edm4hep::MCParticleCollection>();

    parts_in->create(
      0, // std::int32_t type
      0.5, // float energy
      edm4hep::Vector3f({0.5, 0., 0.}), // edm4hep::Vector3f momentum
      edm4hep::Vector3f({0., 0., 0.}), // edm4hep::Vector3f referencePoint
      1., // float charge
      0., // float mass
      0., // float goodnessOfPID
      edm4eic::Cov4f(), // edm4eic::Cov4f covMatrix
      0 // std::int32_t PDG
    );
    mcparts->create(
      11, // std::int32_t PDG
      0, // std::int32_t generatorStatus
      0, // std::int32_t simulatorStatus
      0., // float charge
      0., // float time
      0., // double mass
      edm4hep::Vector3d(), // edm4hep::Vector3d vertex
      edm4hep::Vector3d(), // edm4hep::Vector3d endpoint
      edm4hep::Vector3f(), // edm4hep::Vector3f momentum
      edm4hep::Vector3f(), // edm4hep::Vector3f momentumAtEndpoint
      edm4hep::Vector3f(), // edm4hep::Vector3f spin
      edm4hep::Vector2i() // edm4hep::Vector2i colorFlow
    );

    auto assoc_in = assocs_in->create();
    assoc_in.setRec((*parts_in)[0]);
    assoc_in.setSim((*mcparts)[0]);

    auto parts_out = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto assocs_out = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto partids_out = std::make_unique<edm4hep::ParticleIDCollection>();
    algo.process({parts_in.get(), assocs_in.get()}, {parts_out.get(), assocs_out.get(), partids_out.get()});

    REQUIRE( (*parts_in).size() == (*parts_out).size() );
    REQUIRE( (*assocs_in).size() == (*assocs_out).size() );
    REQUIRE( (*partids_out).size() == (*partids_out).size() );
  }
}
