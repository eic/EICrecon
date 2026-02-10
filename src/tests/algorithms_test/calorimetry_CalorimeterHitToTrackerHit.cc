// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/Volumes.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/CartesianGridXY.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <gsl/pointers>
#include <cmath>
#include <memory>
#include <string>
#include <utility>

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

using eicrecon::CalorimeterHitToTrackerHit;

TEST_CASE("the algorithm runs", "[CalorimeterHitToTrackerHit]") {
  CalorimeterHitToTrackerHit algo("CalorimeterHitToTrackerHit");

  std::shared_ptr<spdlog::logger> logger =
      spdlog::default_logger()->clone("CalorimeterHitToTrackerHit");
  logger->set_level(spdlog::level::trace);

  algo.level(algorithms::LogLevel(spdlog::level::trace));
  algo.init();

  SECTION("empty input produces empty output") {
    auto calorimeter_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits     = std::make_unique<edm4eic::TrackerHitCollection>();

    algo.process({calorimeter_hits.get()}, {tracker_hits.get()});

    REQUIRE(tracker_hits->size() == 0);
  }

  SECTION("single calorimeter hit with CartesianGridXY segmentation") {
    auto detector = const_cast<dd4hep::Detector*>(algorithms::GeoSvc::instance().detector());
    
    // Create a calorimeter readout with CartesianGridXY segmentation
    std::string readout_name = "TestCalorimeterReadout";
    dd4hep::IDDescriptor id_desc(readout_name, "system:8,x:32:-16,y:-16");
    dd4hep::Readout readout(readout_name);
    readout.setIDDescriptor(id_desc);
    
    // Create CartesianGridXY segmentation with specific grid size
    dd4hep::Segmentation seg("CartesianGridXY", "TestCaloSeg", id_desc.decoder());
    auto* grid_xy = dynamic_cast<dd4hep::DDSegmentation::CartesianGridXY*>(seg.segmentation());
    REQUIRE(grid_xy != nullptr);
    grid_xy->setGridSizeX(2.0 * dd4hep::mm);  // 2mm cell size in X
    grid_xy->setGridSizeY(3.0 * dd4hep::mm);  // 3mm cell size in Y
    readout.setSegmentation(seg);
    
    // Add to detector
    detector->add(id_desc);
    detector->add(readout);

    // Create a DetElement hierarchy
    dd4hep::DetElement world_det = detector->world();
    dd4hep::DetElement calo_det("TestCalorimeter", 100);
    
    // Create a simple box volume
    dd4hep::Box box_shape(100 * dd4hep::mm, 100 * dd4hep::mm, 10 * dd4hep::mm);
    dd4hep::Volume calo_volume("TestCaloVolume", box_shape, detector->material("Air"));
    calo_volume.setSensitive();
    calo_volume.setReadout(readout);
    
    // Place the volume at the origin
    dd4hep::Position pos(0, 0, 0);
    dd4hep::PlacedVolume pv = world_det.volume().placeVolume(calo_volume, pos);
    
    // Set the volume ID to match our system ID
    pv.addPhysVolID("system", 100);
    
    // Attach placement to DetElement
    calo_det.setPlacement(pv);
    
    // Add to world
    world_det.add(calo_det);

    // Reinitialize VolumeManager to pick up the new DetElement
    // This is done by accessing it, which triggers lazy initialization
    detector->volumeManager();

    // Create a calorimeter hit
    auto calorimeter_hits = std::make_unique<edm4eic::CalorimeterHitCollection>();
    auto tracker_hits     = std::make_unique<edm4eic::TrackerHitCollection>();

    // Encode cellID with system=100, x=5, y=7
    auto cell_id = id_desc.encode({{"system", 100}, {"x", 5}, {"y", 7}});
    
    auto hit = calorimeter_hits->create(
        cell_id,                                      // cellID
        1.5,                                          // energy (GeV)
        0.05,                                         // energyError
        10.0,                                         // time (ns)
        0.5,                                          // timeError
        edm4hep::Vector3f(10.0, 21.0, 0.0),          // position (mm)
        edm4hep::Vector3f(2.0, 3.0, 10.0),           // dimension
        0,                                            // sector
        0,                                            // layer
        edm4hep::Vector3f(10.0, 21.0, 0.0)           // local position
    );

    algo.process({calorimeter_hits.get()}, {tracker_hits.get()});

    REQUIRE(tracker_hits->size() == 1);
    
    auto& tracker_hit = (*tracker_hits)[0];
    
    // Check that cellID is copied correctly
    REQUIRE(tracker_hit.getCellID() == cell_id);
    
    // Check that position is copied correctly
    REQUIRE_THAT(tracker_hit.getPosition().x, Catch::Matchers::WithinAbs(10.0, 1e-5));
    REQUIRE_THAT(tracker_hit.getPosition().y, Catch::Matchers::WithinAbs(21.0, 1e-5));
    REQUIRE_THAT(tracker_hit.getPosition().z, Catch::Matchers::WithinAbs(0.0, 1e-5));
    
    // Check that time and energy are copied correctly
    REQUIRE_THAT(tracker_hit.getTime(), Catch::Matchers::WithinAbs(10.0, 1e-5));
    REQUIRE_THAT(tracker_hit.getTimeError(), Catch::Matchers::WithinAbs(0.5, 1e-5));
    REQUIRE_THAT(tracker_hit.getEdep(), Catch::Matchers::WithinAbs(1.5, 1e-5));
    REQUIRE_THAT(tracker_hit.getEdepError(), Catch::Matchers::WithinAbs(0.05, 1e-5));
    
    // Check position uncertainties: sigma = dimension / sqrt(12)
    // For 2mm x-cell: sigma_x = 2.0 / sqrt(12) = 0.577... mm
    // For 3mm y-cell: sigma_y = 3.0 / sqrt(12) = 0.866... mm
    // Covariance = sigma^2
    const double expected_cov_xx = (2.0 / std::sqrt(12.0)) * (2.0 / std::sqrt(12.0));
    const double expected_cov_yy = (3.0 / std::sqrt(12.0)) * (3.0 / std::sqrt(12.0));
    
    REQUIRE_THAT(tracker_hit.getCovMatrix().xx, Catch::Matchers::WithinAbs(expected_cov_xx, 1e-5));
    REQUIRE_THAT(tracker_hit.getCovMatrix().yy, Catch::Matchers::WithinAbs(expected_cov_yy, 1e-5));
    REQUIRE_THAT(tracker_hit.getCovMatrix().zz, Catch::Matchers::WithinAbs(0.0, 1e-5));
  }
}
