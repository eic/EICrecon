// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <DD4hep/Detector.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <JANA/Services/JServiceLocator.h>
#include <spdlog/logger.h>
#include <memory>
#include <mutex>

class ActsGeometryProvider;

class ACTSGeo_service : public JService {
public:
  ACTSGeo_service(JApplication* app) : m_app(app) {}
  virtual ~ACTSGeo_service();

  virtual std::shared_ptr<const ActsGeometryProvider> actsGeoProvider();

protected:
private:
  ACTSGeo_service() = default;
  void acquire_services(JServiceLocator*) override;

  std::once_flag m_init_flag;
  JApplication* m_app                 = nullptr;
  const dd4hep::Detector* m_dd4hepGeo = nullptr;
  std::shared_ptr<ActsGeometryProvider> m_acts_provider;

  // General acts log
  std::shared_ptr<spdlog::logger> m_log;
};
