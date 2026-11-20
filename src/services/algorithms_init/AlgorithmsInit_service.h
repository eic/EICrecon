// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, 2024 Wouter Deconinck, Sylvester Joosten

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <spdlog/logger.h>
#include <memory>

class DD4hep_service;
class Log_service;

/**
 * The AlgorithmsInit_service centralizes use of ServiceSvc
 */
class AlgorithmsInit_service : public JService {
public:
  explicit AlgorithmsInit_service(JApplication* /* app */) {}
  ~AlgorithmsInit_service() override = default;

  // Defined inline so that the class has no key function: its typeinfo is then emitted in every
  // plugin that uses Service<AlgorithmsInit_service>, instead of only in this plugin
  void acquire_services(JServiceLocator* srv_locator) override { register_services(srv_locator); }

private:
  AlgorithmsInit_service() = default;
  void register_services(JServiceLocator* srv_locator);
  std::shared_ptr<Log_service> m_log_service;
  std::shared_ptr<DD4hep_service> m_dd4hep_service;
  std::shared_ptr<spdlog::logger> m_log;
};
