// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <algorithms/geo.h>
#include <algorithms/random.h>
#include <algorithms/service.h>
#include <algorithms/interfaces/ParticleSvc.h>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <services/evaluator/EvaluatorSvc.h>
#include <services/pid_lut/PIDLookupTableSvc.h>
#include <stddef.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

class algorithmsInitListener : public Catch::EventListenerBase {
public:
  using Catch::EventListenerBase::EventListenerBase;

  std::unique_ptr<const dd4hep::Detector> m_detector{nullptr};

  void testRunStarting(Catch::TestRunInfo const&) override {
    auto detector = dd4hep::Detector::make_unique("");
    dd4hep::Readout readout(std::string("MockCalorimeterHits"));
    dd4hep::IDDescriptor id_desc("MockCalorimeterHits", "system:8,layer:8,x:8,y:8");
    readout.setIDDescriptor(id_desc);
    detector->add(id_desc);
    detector->add(readout);

    dd4hep::Readout readoutTracker(std::string("MockTrackerHits"));
    dd4hep::IDDescriptor id_desc_tracker("MockTrackerHits", "system:8,layer:8,x:8,y:8");
    //Create segmentation with 1x1 mm pixels
    dd4hep::Segmentation segmentation("CartesianGridXY","TrackerHitsSeg", id_desc_tracker.decoder());
    readoutTracker.setIDDescriptor(id_desc_tracker);
    readoutTracker.setSegmentation(segmentation);
    detector->add(id_desc_tracker);
    detector->add(readoutTracker);

    m_detector = std::move(detector);

    auto& serviceSvc = algorithms::ServiceSvc::instance();
    [[maybe_unused]] auto& geoSvc = algorithms::GeoSvc::instance();
    serviceSvc.setInit<algorithms::GeoSvc>([this](auto&& g) {
      g.init(this->m_detector.get());
    });

    [[maybe_unused]] auto& randomSvc = algorithms::RandomSvc::instance();
    auto seed = Catch::Generators::Detail::getSeed();
    serviceSvc.setInit<algorithms::RandomSvc>([seed](auto&& r) {
      r.setProperty("seed", static_cast<size_t>(seed));
      r.init();
    });

    auto& evaluatorSvc = eicrecon::EvaluatorSvc::instance();
    serviceSvc.add<eicrecon::EvaluatorSvc>(&evaluatorSvc);

    auto& lutSvc = eicrecon::PIDLookupTableSvc::instance();
    serviceSvc.add<eicrecon::PIDLookupTableSvc>(&lutSvc);

    auto& particleSvc = algorithms::ParticleSvc::instance();
    serviceSvc.add<algorithms::ParticleSvc>(&particleSvc);

    serviceSvc.init();
  }
};

CATCH_REGISTER_LISTENER(algorithmsInitListener)
