// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <algorithms/random.h>
#include <algorithms/service.h>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
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
    m_detector = std::move(detector);

    auto& serviceSvc              = algorithms::ServiceSvc::instance();
    [[maybe_unused]] auto& geoSvc = algorithms::GeoSvc::instance();
    serviceSvc.setInit<algorithms::GeoSvc>([this](auto&& g) { g.init(this->m_detector.get()); });

    [[maybe_unused]] auto& randomSvc = algorithms::RandomSvc::instance();
    auto seed                        = Catch::Generators::Detail::getSeed();
    serviceSvc.setInit<algorithms::RandomSvc>([seed](auto&& r) {
      r.setProperty("seed", static_cast<size_t>(seed));
      r.init();
    });

    serviceSvc.init();
  }
};

CATCH_REGISTER_LISTENER(algorithmsInitListener)
