// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, 2024 Wouter Deconinck, Sylvester Joosten

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JService.h>
#include <JANA/Services/JServiceLocator.h>
#include <spdlog/logger.h>
#include <memory>

class Log_service;
class DD4hep_service;

/**
 * The AlgorithmsInit_service centralizes use of ServiceSvc
 */
class AlgorithmsInit_service : public JService {
public:
  explicit AlgorithmsInit_service(JApplication* /* app */) {}
  ~AlgorithmsInit_service() override = default;

  void acquire_services(JServiceLocator* srv_locator) override;

private:
  AlgorithmsInit_service() = default;
  std::shared_ptr<Log_service> m_log_service;
  std::shared_ptr<DD4hep_service> m_dd4hep_service;
  std::shared_ptr<spdlog::logger> m_log;
};
