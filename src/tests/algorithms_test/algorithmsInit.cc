// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/Volumes.h>
#include <DDSegmentation/CartesianGridXY.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <algorithms/interfaces/UniqueIDGenSvc.h>
#include <services/particle/ParticleSvc.h>
#include <algorithms/random.h>
#include <algorithms/service.h>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <services/evaluator/EvaluatorSvc.h>
#include <services/pid_lut/PIDLookupTableSvc.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

class algorithmsInitListener : public Catch::EventListenerBase {
public:
  using Catch::EventListenerBase::EventListenerBase;

  std::unique_ptr<const dd4hep::Detector> m_detector{nullptr};

  void testRunStarting(Catch::TestRunInfo const& /*testRunInfo*/) override {
    auto detector = dd4hep::Detector::make_unique("");
    detector->addConstant(dd4hep::Constant("MockCalorimeter_ID", "1"));
    dd4hep::Readout readout(std::string("MockCalorimeterHits"));
    dd4hep::IDDescriptor id_desc("MockCalorimeterHits", "system:8,layer:8,x:8,y:8");
    readout.setIDDescriptor(id_desc);
    detector->add(id_desc);
    detector->add(readout);

    detector->addConstant(dd4hep::Constant("MockTracker_ID", "2"));
    dd4hep::Readout readoutTracker(std::string("MockTrackerHits"));
    dd4hep::IDDescriptor id_desc_tracker("MockTrackerHits", "system:8,layer:8,x:8,y:8");
    //Create segmentation with 1x1 mm pixels
    dd4hep::Segmentation segmentation("CartesianGridXY", "TrackerHitsSeg",
                                      id_desc_tracker.decoder());
    readoutTracker.setIDDescriptor(id_desc_tracker);
    readoutTracker.setSegmentation(segmentation);
    detector->add(id_desc_tracker);
    detector->add(readoutTracker);

    dd4hep::Readout readoutSilicon(std::string("MockSiliconHits"));
    dd4hep::IDDescriptor id_desc_Silicon("MockSiliconHits",
                                         "system:8,layer:4,module:12,sensor:10,x:40:-8,y:-16");
    //Create segmentation with 1x1 mm pixels
    dd4hep::Segmentation segmentation_Silicon("CartesianGridXY", "SiliconHitsSeg",
                                              id_desc_tracker.decoder());
    readoutSilicon.setIDDescriptor(id_desc_Silicon);
    readoutSilicon.setSegmentation(segmentation_Silicon);
    detector->add(id_desc_Silicon);
    detector->add(readoutSilicon);

    // Add test calorimeter readout for CalorimeterHitToTrackerHit test
    detector->addConstant(dd4hep::Constant("TestCalorimeter_ID", "100"));
    dd4hep::Readout readoutTestCalo(std::string("TestCalorimeterReadout"));
    dd4hep::IDDescriptor id_desc_test_calo("TestCalorimeterReadout", "system:8,x:32:-16,y:-16");
    dd4hep::Segmentation segmentation_test_calo("CartesianGridXY", "TestCaloSeg",
                                                id_desc_test_calo.decoder());
    auto* grid_xy = dynamic_cast<dd4hep::DDSegmentation::CartesianGridXY*>(
        segmentation_test_calo.segmentation());
    if (grid_xy) {
      grid_xy->setGridSizeX(2.0 * dd4hep::mm); // 2mm cell size in X
      grid_xy->setGridSizeY(3.0 * dd4hep::mm); // 3mm cell size in Y
    }
    readoutTestCalo.setIDDescriptor(id_desc_test_calo);
    readoutTestCalo.setSegmentation(segmentation_test_calo);
    detector->add(id_desc_test_calo);
    detector->add(readoutTestCalo);

    // Create DetElement hierarchy for test calorimeter
    dd4hep::DetElement world_det("world", 0);
    dd4hep::Box world_box(1000 * dd4hep::mm, 1000 * dd4hep::mm, 1000 * dd4hep::mm);
    dd4hep::Volume world_volume("world_volume", world_box, detector->material("Air"));
    detector->setWorldVolume(world_volume);
    world_det.setPlacement(dd4hep::PlacedVolume(world_volume));

    dd4hep::DetElement calo_det("TestCalorimeter", 100);
    dd4hep::Box box_shape(100 * dd4hep::mm, 100 * dd4hep::mm, 10 * dd4hep::mm);
    dd4hep::Volume calo_volume("TestCaloVolume", box_shape, detector->material("Air"));
    calo_volume.setSensitive();
    calo_volume.setReadout(readoutTestCalo);

    dd4hep::Position pos(0, 0, 0);
    dd4hep::PlacedVolume pv = world_volume.placeVolume(calo_volume, pos);
    pv.addPhysVolID("system", 100);
    calo_det.setPlacement(pv);
    world_det.add(calo_det);

    m_detector = std::move(detector);

    auto& serviceSvc              = algorithms::ServiceSvc::instance();
    [[maybe_unused]] auto& geoSvc = algorithms::GeoSvc::instance();
    serviceSvc.setInit<algorithms::GeoSvc>([this](auto&& g) { g.init(this->m_detector.get()); });

    [[maybe_unused]] auto& randomSvc = algorithms::RandomSvc::instance();
    auto seed                        = Catch::Generators::Detail::getSeed();
    serviceSvc.setInit<algorithms::RandomSvc>([seed](auto&& r) {
      r.setProperty("seed", static_cast<std::size_t>(seed));
      r.init();
    });

    auto& evaluatorSvc = eicrecon::EvaluatorSvc::instance();
    serviceSvc.add<eicrecon::EvaluatorSvc>(&evaluatorSvc);

    auto& lutSvc = eicrecon::PIDLookupTableSvc::instance();
    serviceSvc.add<eicrecon::PIDLookupTableSvc>(&lutSvc);

    auto& particleSvc = algorithms::ParticleSvc::instance();
    serviceSvc.add<algorithms::ParticleSvc>(&particleSvc);

    auto& uniqueIDSvc = algorithms::UniqueIDGenSvc::instance();
    serviceSvc.add<algorithms::UniqueIDGenSvc>(&uniqueIDSvc);

    serviceSvc.init();
  }
};

CATCH_REGISTER_LISTENER(algorithmsInitListener)
