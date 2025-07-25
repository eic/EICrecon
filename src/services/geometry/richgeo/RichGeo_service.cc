// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichGeo_service.h"

#include <JANA/JException.h>
#include <fmt/core.h>
#include <exception>
#include <gsl/pointers>

#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/geometry/richgeo/ActsGeo.h"
#include "services/geometry/richgeo/IrtGeo.h"
#include "services/geometry/richgeo/IrtGeoDRICH.h"
#include "services/geometry/richgeo/IrtGeoPFRICH.h"
#include "services/geometry/richgeo/ReadoutGeo.h"
#include "services/log/Log_service.h"

// Services ----------------------------------------------------------
void RichGeo_service::acquire_services(JServiceLocator* srv_locator) {

  // logging service
  auto log_service = srv_locator->get<Log_service>();
  m_log            = log_service->logger("richgeo");

  // DD4Hep geometry service
  auto dd4hep_service = srv_locator->get<DD4hep_service>();
  m_dd4hepGeo         = dd4hep_service->detector();
  m_converter         = dd4hep_service->converter();
}

// IrtGeo -----------------------------------------------------------
richgeo::IrtGeo* RichGeo_service::GetIrtGeo(std::string detector_name) {

  // initialize, if not yet initialized
  try {
    m_log->debug("Call RichGeo_service::GetIrtGeo initializer");
    auto initialize = [this, &detector_name]() {
      if (m_dd4hepGeo == nullptr) {
        throw JException("RichGeo_service m_dd4hepGeo==null which should never be!");
      }
      // instantiate IrtGeo-derived object, depending on detector
      auto which_rich = detector_name;
      if (which_rich == "DRICH") {
        m_irtGeo = new richgeo::IrtGeoDRICH(m_dd4hepGeo, m_converter, m_log);
      } else if (which_rich == "RICHEndcapN") {
        m_irtGeo = new richgeo::IrtGeoPFRICH(m_dd4hepGeo, m_converter, m_log);
      } else {
        throw JException(fmt::format("IrtGeo is not defined for detector '{}'", detector_name));
      }
    };
    std::lock_guard<std::mutex> lock(m_init_lock);
    std::call_once(m_init_irt[detector_name], initialize);
  } catch (std::exception& ex) {
    throw JException(ex.what());
  }

  return m_irtGeo;
}

// ActsGeo -----------------------------------------------------------
const richgeo::ActsGeo* RichGeo_service::GetActsGeo(std::string detector_name) {
  // initialize, if not yet initialized
  try {
    m_log->debug("Call RichGeo_service::GetActsGeo initializer");
    auto initialize = [this, &detector_name]() {
      if (m_dd4hepGeo == nullptr) {
        throw JException("RichGeo_service m_dd4hepGeo==null which should never be!");
      }
      m_actsGeo = new richgeo::ActsGeo(detector_name, m_dd4hepGeo, m_log);
    };
    std::lock_guard<std::mutex> lock(m_init_lock);
    std::call_once(m_init_acts[detector_name], initialize);
  } catch (std::exception& ex) {
    throw JException(ex.what());
  }
  return m_actsGeo;
}

// ReadoutGeo -----------------------------------------------------------
std::shared_ptr<richgeo::ReadoutGeo> RichGeo_service::GetReadoutGeo(std::string detector_name,
                                                                    std::string readout_class) {
  // initialize, if not yet initialized
  try {
    m_log->debug("Call RichGeo_service::GetReadoutGeo initializer");
    auto initialize = [this, &detector_name, &readout_class]() {
      if (m_dd4hepGeo == nullptr) {
        throw JException("RichGeo_service m_dd4hepGeo==null which should never be!");
      }
      m_readoutGeo = std::make_shared<richgeo::ReadoutGeo>(detector_name, readout_class,
                                                           m_dd4hepGeo, m_converter, m_log);
    };
    std::lock_guard<std::mutex> lock(m_init_lock);
    std::call_once(m_init_readout[detector_name], initialize);
  } catch (std::exception& ex) {
    throw JException(ex.what());
  }
  return m_readoutGeo;
}

// Destructor --------------------------------------------------------
RichGeo_service::~RichGeo_service() {
  try {
    delete m_irtGeo;
    delete m_actsGeo;
  } catch (...) {
  }
}
