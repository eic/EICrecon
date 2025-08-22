// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, 2024 Wouter Deconinck, Sylvester Joosten

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JServiceLocator.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <algorithms/random.h>
#include <algorithms/service.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>

#include "algorithms/interfaces/ActsSvc.h"
#include "services/particle/ParticleSvc.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "services/log/Log_service.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

/**
 * The AlgorithmsInit_service centralizes use of ServiceSvc
 */
class AlgorithmsInit_service : public JService {
public:
  AlgorithmsInit_service(JApplication* /* app */) {};
  virtual ~AlgorithmsInit_service() {};

  void acquire_services(JServiceLocator* srv_locator) override {
    auto& serviceSvc = algorithms::ServiceSvc::instance();

    // Get services
    m_log_service     = srv_locator->get<Log_service>();
    m_dd4hep_service  = srv_locator->get<DD4hep_service>();
    m_actsgeo_service = srv_locator->get<ACTSGeo_service>();

    // Logger for ServiceSvc
    m_log = m_log_service->logger("AlgorithmsInit");

    // Register DD4hep_service as algorithms::GeoSvc
    [[maybe_unused]] auto& geoSvc = algorithms::GeoSvc::instance();
    serviceSvc.setInit<algorithms::GeoSvc>([this](auto&& g) {
      this->m_log->debug("Initializing algorithms::GeoSvc");
      g.init(const_cast<dd4hep::Detector*>(this->m_dd4hep_service->detector().get()));
    });

    // Register DD4hep_service as algorithms::ActsSvc
    [[maybe_unused]] auto& actsSvc = algorithms::ActsSvc::instance();
    serviceSvc.setInit<algorithms::ActsSvc>([this](auto&& g) {
      this->m_log->debug("Initializing algorithms::ActsSvc");
      try {
        g.init(this->m_actsgeo_service->actsGeoProvider());
      } catch (...) {
        g.init(std::move(std::current_exception()));
      }
    });

    // Register Log_service as algorithms::LogSvc
    const algorithms::LogLevel level{static_cast<algorithms::LogLevel>(m_log->level())};
    serviceSvc.setInit<algorithms::LogSvc>([this, level](auto&& logger) {
      this->m_log->debug("Initializing algorithms::LogSvc");
      logger.init(
          [this](const algorithms::LogLevel l, std::string_view caller, std::string_view msg) {
            static std::mutex m;
            std::lock_guard<std::mutex> lock(m);
            // storing the string_view is unsafe since it can become invalid
            static std::map<std::string, std::shared_ptr<spdlog::logger>> loggers;
            if (!loggers.contains(std::string(caller))) {
              this->m_log->debug("Initializing algorithms::LogSvc logger {}", caller);
              loggers[std::string(caller)] = this->m_log_service->logger(std::string(caller));
            }
            loggers[std::string(caller)]->log(static_cast<spdlog::level::level_enum>(l), msg);
          });
      logger.defaultLevel(level);
    });

    // Register a random service (JANA2 does not have one)
    [[maybe_unused]] auto& randomSvc = algorithms::RandomSvc::instance();
    serviceSvc.setInit<algorithms::RandomSvc>([this](auto&& r) {
      this->m_log->debug("Initializing algorithms::RandomSvc");
      r.setProperty("seed", static_cast<std::size_t>(1));
      r.init();
    });

    // Register a particle service
    [[maybe_unused]] auto& particleSvc = algorithms::ParticleSvc::instance();
    serviceSvc.add<algorithms::ParticleSvc>(&particleSvc);

    // Register a unique ID service
    [[maybe_unused]] auto& uniqueIDGenSvc = algorithms::UniqueIDGenSvc::instance();
    for (const auto& [key, prop] : uniqueIDGenSvc.getProperties()) {
      std::visit(
          [this, &uniqueIDGenSvc, key = key](auto&& val) {
            this->GetApplication()->SetDefaultParameter(std::string(key),
                                                        val); // FIXME add description
            uniqueIDGenSvc.setProperty(key, val);
          },
          prop.get());
    }
    serviceSvc.add<algorithms::UniqueIDGenSvc>(&uniqueIDGenSvc);

    // Finally, initialize the ServiceSvc
    serviceSvc.init();
  }

private:
  AlgorithmsInit_service() = default;
  std::shared_ptr<Log_service> m_log_service;
  std::shared_ptr<DD4hep_service> m_dd4hep_service;
  std::shared_ptr<ACTSGeo_service> m_actsgeo_service;
  std::shared_ptr<spdlog::logger> m_log;
};
