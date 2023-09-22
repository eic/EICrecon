// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichGeo_service.h"

// Services ----------------------------------------------------------
void RichGeo_service::acquire_services(JServiceLocator *srv_locator) {

  // logging service
  auto log_service = srv_locator->get<Log_service>();
  m_log = log_service->logger("richgeo");
  std::string log_level_str = "info";
  m_app->SetDefaultParameter("richgeo:LogLevel", log_level_str, "Log level for RichGeo_service");
  m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
  m_log->debug("RichGeo log level is set to {} ({})", log_level_str, m_log->level());

  // DD4Hep geometry service
  auto dd4hep_service = srv_locator->get<DD4hep_service>();
  m_dd4hepGeo = dd4hep_service->detector();
}

// IrtGeo -----------------------------------------------------------
richgeo::IrtGeo *RichGeo_service::GetIrtGeo(std::string detector_name) {

  // initialize, if not yet initialized
  try {
    m_log->debug("Call RichGeo_service::GetIrtGeo initializer");
    auto initialize = [this,&detector_name] () {
      if(!m_dd4hepGeo) throw JException("RichGeo_service m_dd4hepGeo==null which should never be!");
      // instantiate IrtGeo-derived object, depending on detector
      auto which_rich = detector_name;
      std::transform(which_rich.begin(), which_rich.end(), which_rich.begin(), ::toupper);
      if     ( which_rich=="DRICH"  ) m_irtGeo = new richgeo::IrtGeoDRICH(m_dd4hepGeo,  m_log);
      else if( which_rich=="PFRICH" ) m_irtGeo = new richgeo::IrtGeoPFRICH(m_dd4hepGeo, m_log);
      else throw JException(fmt::format("IrtGeo is not defined for detector '{}'",detector_name));
    };
    std::call_once(m_init_irt, initialize);
  }
  catch (std::exception &ex) {
    throw JException(ex.what());
  }

  return m_irtGeo;
}

// ActsGeo -----------------------------------------------------------
richgeo::ActsGeo *RichGeo_service::GetActsGeo(std::string detector_name) {
  // initialize, if not yet initialized
  try {
    m_log->debug("Call RichGeo_service::GetActsGeo initializer");
    auto initialize = [this,&detector_name] () {
      if(!m_dd4hepGeo) throw JException("RichGeo_service m_dd4hepGeo==null which should never be!");
      m_actsGeo = new richgeo::ActsGeo(detector_name, m_dd4hepGeo, m_log);
    };
    std::call_once(m_init_acts, initialize);
  }
  catch (std::exception &ex) {
    throw JException(ex.what());
  }
  return m_actsGeo;
}

// ReadoutGeo -----------------------------------------------------------
std::shared_ptr<richgeo::ReadoutGeo> RichGeo_service::GetReadoutGeo(std::string detector_name) {
  // initialize, if not yet initialized
  try {
    m_log->debug("Call RichGeo_service::GetReadoutGeo initializer");
    auto initialize = [this,&detector_name] () {
      if(!m_dd4hepGeo) throw JException("RichGeo_service m_dd4hepGeo==null which should never be!");
      m_readoutGeo = std::make_shared<richgeo::ReadoutGeo>(detector_name, m_dd4hepGeo, m_log);
    };
    std::call_once(m_init_readout, initialize);
  }
  catch (std::exception &ex) {
    throw JException(ex.what());
  }
  return m_readoutGeo;
}

// Destructor --------------------------------------------------------
RichGeo_service::~RichGeo_service() {
  try {
    if(m_dd4hepGeo) m_dd4hepGeo->destroyInstance();
    m_dd4hepGeo = nullptr;
    delete m_irtGeo;
    delete m_actsGeo;
  } catch (...) {}
}
