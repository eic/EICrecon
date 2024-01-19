// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Dmitry Kalinkin

#include "DD4hep/DetFactoryHelper.h"
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include "DD4hep/VolumeManager.h"
#include <Evaluator/DD4hepUnits.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/HEXPLIT.h"
#include "algorithms/calorimetry/HEXPLITConfig.h"

using eicrecon::HEXPLIT;
using eicrecon::HEXPLITConfig;

TEST_CASE( "the subcell-splitting algorithm runs", "[HEXPLIT]" ) {
  HEXPLIT algo("HEXPLIT");
  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("HEXPLIT");
  logger->set_level(spdlog::level::trace);
  HEXPLITConfig cfg;
  cfg.MIP = 472. * dd4hep::keV;
  cfg.tmax = 1000. * dd4hep::ns;
  std::cout << "creating mock detector"<<std::endl;
  auto detector = dd4hep::Detector::make_unique("");
  std::cout << "created mock detector"<<std::endl;
  dd4hep::Readout readout(std::string("MockCalorimeterHits"));
  std::cout << "created mock detector readout"<<std::endl;
  dd4hep::IDDescriptor id_desc("MockCalorimeterHits", "system:8,layer:8,x:8,y:8");
  std::cout << "created mock detector readout id_desc"<<std::endl;
  readout.setIDDescriptor(id_desc);
  std::cout << "set readout's id_desc"<<std::endl;
  detector->add(id_desc);
  std::cout << "added id_desc to detector"<<std::endl;
  detector->add(readout);
  std::cout << "added readout to detector"<<std::endl;






  //create a geometry for the fake detector.

  double side_length=31.3*dd4hep::mm;
  double layer_spacing=25.1*dd4hep::mm;
  double thickness=3*dd4hep::mm;

//  int detID=0;
//
//  //I'm not sure how much of this is necessary
//  dd4hep::Box envelope(10*side_length, 10*side_length, layer_spacing*10);
//  dd4hep::Material   mat        = detector->material("Pb");
//  // Defining envelope volume
//  dd4hep::Volume envelopeVol("MockDetector", envelope, mat);
//
//  dd4hep::DetElement   det("MockDetector", detID);
//  dd4hep::Volume motherVol = detector->pickMotherVolume(det);
//
//  // Placing detector in world volume
//  auto tr = dd4hep::Transform3D(dd4hep::RotationZYX(0, 0, 0),dd4hep::Position(0, 0, 0));
//  dd4hep::PlacedVolume phv = motherVol.placeVolume(envelopeVol, tr);
//  phv.addPhysVolID("system", detID);
//  det.setPlacement(phv);

  //dimension of a cell
  auto dimension = edm4hep::Vector3f(2*side_length, sqrt(3)*side_length, thickness);

  algo.applyConfig(cfg);
  std::cout << "applied config to algo"<<std::endl;
  algo.init(detector.get(), logger);
  std::cout << "initiated algo"<<std::endl;


  edm4eic::CalorimeterHitCollection hits_coll;

  //create a set of 5 hits in consecutive layers, all of which overlap in a single rhombus,
  // centered at (3/8, sqrt(3)/8)*side_length
  std::array<double,5> layer={0,1,2,3,4};
  std::array<double,5> x={0,0.75*side_length,0,0.75*side_length,0};
  std::array<double,5> y={sqrt(3)/2*side_length,-0.25*sqrt(3)*side_length,0,0.25*sqrt(3)*side_length,sqrt(3)/2*side_length};
  for(size_t i=0; i<5; i++){
    hits_coll.create(
                     id_desc.encode({{"system", 255}, {"x", 0}, {"y", 0}}), // std::uint64_t cellID,
                     5.0*dd4hep::MeV, // float energy,
                     0.0, // float energyError,
                     0.0, // float time,
                     0.0, // float timeError,
                     edm4hep::Vector3f(x[i], y[i], layer[i]*layer_spacing), // edm4hep::Vector3f position,
                     dimension, // edm4hep::Vector3f dimension,
                     0, // std::int32_t sector,
                     layer[i], // std::int32_t layer,
                     edm4hep::Vector3f(x[i], y[i], layer[i]*layer_spacing) // edm4hep::Vector3f local
                     );
  }
  std::cout << "created input hits"<<std::endl;

  //auto context = new dd4hep::VolumeManagerContext;
  //std::cout <<"created volume manager context" << std::endl;
  //detector->volumeManager().adoptPlacement(0,context);
  //std::cout <<"applied volume manager context" << std::endl;

  auto subcellhits_coll = std::make_unique<edm4eic::CalorimeterHitCollection>();
  //edm4eic::CalorimeterHitCollection subcellhits_coll;
  std::cout << "created output subcell hits collection"<<std::endl;
  algo.process({&hits_coll}, {subcellhits_coll.get()});
  std::cout << "processed hits;  subcells found=" << subcellhits_coll->size() << std::endl;
  for(auto subcell : *subcellhits_coll){
    std::cout << subcell.getLayer() << " " << subcell.getLocal().x/side_length << " " << subcell.getLocal().z/side_length << " " << subcell.getLocal().z/side_length << " " << subcell.getEnergy() << std::endl;

  }

  //the number of subcell hits should be equal to the
  //number of subcells per cell (12) times the number of cells (5)
  REQUIRE( (*subcellhits_coll).size() == 60);
  //REQUIRE( (*protoclust_coll)[0].hits_size() == 1 );
  //REQUIRE( (*protoclust_coll)[0].weights_size() == 1 );


}
