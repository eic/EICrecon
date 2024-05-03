// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Sebouh Paul

#include <DD4hep/Detector.h>                       // for Detector
#include <DD4hep/IDDescriptor.h>                   // for IDDescriptor
#include <DD4hep/Readout.h>                        // for Readout
#include <Evaluator/DD4hepUnits.h>                 // for MeV, mm, keV, ns
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>            // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <edm4eic/ClusterCollection.h>
#include <edm4hep/Vector3f.h>                      // for Vector3f
#include <spdlog/common.h>                         // for level_enum
#include <spdlog/logger.h>                         // for logger
#include <spdlog/spdlog.h>                         // for default_logger
#include <stddef.h>                                // for size_t
#include <array>                                   // for array
#include <cmath>                                   // for sqrt, abs
#include <gsl/pointers>
#include <memory>                                  // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <utility>                                 // for pair

#include "algorithms/calorimetry/NeutronReconstruction.h"        // for Neutronreconstruction
#include "algorithms/calorimetry/NeutronReconstructionConfig.h"  // for NeutronreconstructionConfig

using eicrecon::NeutronReconstruction;
using eicrecon::NeutronReconstructionConfig;

TEST_CASE( "the cluster merging algorithm runs", "[NeutronReconstruction]" ) {
  NeutronReconstruction algo("NeutronReconstruction");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("NeutronReconstruction");
  logger->set_level(spdlog::level::trace);

  NeutronReconstructionConfig cfg;

  algo.applyConfig(cfg);
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
  double tol=0.001;
  double E_expected=90*dd4hep::GeV;
  double Px_expected=0.08999;
  double Py_expected=-0.08999;
  double Pz_expected=89.99;
  //check that the correct energy and momenta are being obtained
  std::cout << (*neutroncand_coll)[0].getEnergy() <<"  " << (*neutroncand_coll)[0].getMomentum().x << "  " << (*neutroncand_coll)[0].getMomentum().y << "  " << (*neutroncand_coll)[0].getMomentum().z << std::endl;
  REQUIRE( abs((*neutroncand_coll)[0].getEnergy()-E_expected)/E_expected<tol);
  REQUIRE( abs((*neutroncand_coll)[0].getMomentum().x-Px_expected)/Px_expected<tol);
  REQUIRE( abs((*neutroncand_coll)[0].getMomentum().y-Py_expected)/Py_expected<tol);
  REQUIRE( abs((*neutroncand_coll)[0].getMomentum().z-Pz_expected)/Pz_expected<tol);

}
