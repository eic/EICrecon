// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include <algorithms/logger.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4eic/Cov4f.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/EDM4hepVersion.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#if EDM4HEP_BUILD_VERSION < EDM4HEP_VERSION(0, 99, 2)
#include <edm4hep/Vector2i.h>
#endif
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/pid_lut/PIDLookup.h"
#include "algorithms/pid_lut/PIDLookupConfig.h"

using eicrecon::PIDLookup;
using eicrecon::PIDLookupConfig;

TEST_CASE("particles acquire PID", "[PIDLookup]") {
  PIDLookup algo("test");

  PIDLookupConfig cfg{
      .filename                    = "/dev/null",
      .system                      = "MockTracker_ID",
      .pdg_values                  = {11},
      .charge_values               = {1},
      .momentum_edges              = {0., 1., 2.},
      .polar_edges                 = {0., M_PI},
      .azimuthal_binning           = {0., 2 * M_PI, 2 * M_PI}, // lower, upper, step
      .momentum_bin_centers_in_lut = true,
      .polar_bin_centers_in_lut    = true,
      .use_radians                 = true,
  };

  SECTION("single hit with couple contributions") {
    algo.level(algorithms::LogLevel(spdlog::level::trace));
    algo.applyConfig(cfg);
    algo.init();

    auto headers = std::make_unique<edm4hep::EventHeaderCollection>();
    auto header  = headers->create(1, 1, 12345678, 1.0);

    auto parts_in  = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto assocs_in = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto mcparts   = std::make_unique<edm4hep::MCParticleCollection>();

    parts_in->create(0,                                // std::int32_t type
                     0.5,                              // float energy
                     edm4hep::Vector3f({0.5, 0., 0.}), // edm4hep::Vector3f momentum
                     edm4hep::Vector3f({0., 0., 0.}),  // edm4hep::Vector3f referencePoint
                     1.,                               // float charge
                     0.,                               // float mass
                     0.,                               // float goodnessOfPID
                     edm4eic::Cov4f(),                 // edm4eic::Cov4f covMatrix
                     0                                 // std::int32_t PDG
    );
    mcparts->create(11,                  // std::int32_t PDG
                    0,                   // std::int32_t generatorStatus
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

    auto assoc_in = assocs_in->create();
    assoc_in.setRec((*parts_in)[0]);
    assoc_in.setSim((*mcparts)[0]);

    auto parts_out   = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    auto assocs_out  = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
    auto partids_out = std::make_unique<edm4hep::ParticleIDCollection>();
    algo.process({headers.get(), parts_in.get(), assocs_in.get()},
                 {parts_out.get(), assocs_out.get(), partids_out.get()});

    REQUIRE((*parts_in).size() == (*parts_out).size());
    REQUIRE((*assocs_in).size() == (*assocs_out).size());
    REQUIRE(
        0 ==
        (*partids_out).size()); // Since our table is empty, there will not be a successful lookup
  }
}
