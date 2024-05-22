// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul

#include <Evaluator/DD4hepUnits.h>                 // for MeV, mm, keV, ns
#include <catch2/catch_test_macros.hpp>            // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>                      // for Vector3f
#include <spdlog/common.h>                         // for level_enum
#include <spdlog/logger.h>                         // for logger
#include <spdlog/spdlog.h>                         // for default_logger
#include <stddef.h>                                // for size_t
#include <stdlib.h>
#include <array>                                   // for array
#include <cmath>                                   // for sqrt, abs
#include <iostream>
#include <memory>                                  // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access

#include "algorithms/reco/FarForwardNeutronReconstruction.h"        // for Neutronreconstruction

using eicrecon::FarForwardNeutronReconstruction;

TEST_CASE( "the cluster merging algorithm runs", "[FarForwardNeutronReconstruction]" ) {
  FarForwardNeutronReconstruction algo("FarForwardNeutronReconstruction");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("FarForwardNeutronReconstruction");
  logger->set_level(spdlog::level::trace);

  algo.init();

  edm4eic::ClusterCollection clust_coll;

  std::array<float,3> x={30*dd4hep::mm,90*dd4hep::mm,0};
  std::array<float,3> y={-30*dd4hep::mm,0*dd4hep::mm, -90*dd4hep::mm};
  std::array<float,3> z={30*dd4hep::m,30*dd4hep::m, 30*dd4hep::m};
  std::array<double,3> E={80*dd4hep::GeV,5*dd4hep::GeV,5*dd4hep::GeV};
  float sumEnergies=0;
  for(size_t i=0; i<3; i++){
    auto cluster=clust_coll.create();
    cluster.setEnergy(E[i]);
    cluster.setPosition({x[i], y[i], z[i]});

  }
  auto neutroncand_coll = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  algo.process({&clust_coll}, {neutroncand_coll.get()});

  REQUIRE( (*neutroncand_coll).size() == 1);

  double corr=algo.calc_corr(90);
  double tol=0.001;
  double E_expected=90*dd4hep::GeV*1/(1+corr);
  double Px_expected=0.08999*1/(1+corr);
  double Py_expected=-0.08999*1/(1+corr);
  double Pz_expected=89.99*1/(1+corr);
  //check that the correct energy and momenta are being obtained
  std::cout << "E, px, py, pz = " << (*neutroncand_coll)[0].getEnergy() <<"  " << (*neutroncand_coll)[0].getMomentum().x << "  " << (*neutroncand_coll)[0].getMomentum().y << "  " << (*neutroncand_coll)[0].getMomentum().z << std::endl;
  REQUIRE( abs((*neutroncand_coll)[0].getEnergy()-E_expected)/E_expected<tol);
  REQUIRE( abs((*neutroncand_coll)[0].getMomentum().x-Px_expected)/Px_expected<tol);
  REQUIRE( abs((*neutroncand_coll)[0].getMomentum().y-Py_expected)/Py_expected<tol);
  REQUIRE( abs((*neutroncand_coll)[0].getMomentum().z-Pz_expected)/Pz_expected<tol);

}
