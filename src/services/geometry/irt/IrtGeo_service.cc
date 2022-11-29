// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeo_service.h"

// Services ----------------------------------------------------------
void IrtGeo_service::acquire_services(JServiceLocator *srv_locator) {

  // logging service
  auto log_service = srv_locator->get<Log_service>();
  m_log = log_service->logger("irt");
  std::string log_level_str = "info";
  m_app->SetDefaultParameter("irt:LogLevel", log_level_str, "Log level for IrtGeo_service");
  m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
  m_log->info("IrtGeo IRT log level is set to {} ({})", log_level_str, m_log->level());

  // DD4Hep geometry service
  auto dd4hep_service = srv_locator->get<JDD4hep_service>();
  m_dd4hepGeo = dd4hep_service->detector();
}

// IrtGeometry -----------------------------------------------------
IrtGeo *IrtGeo_service::GetIrtGeo(std::string detector_name) {

  // initialize, if not yet initialized
  try {
    m_log->info("Call IrtGeo_service::Initialize");
    auto initialize = [this,&detector_name] () {
      if(!m_dd4hepGeo) throw JException("IrtGeo_service m_dd4hepGeo==null which should never be!");
      // allow verbose IrtGeo output, if log level is `debug` or lower
      bool verbose = m_log->level() <= spdlog::level::debug;
      // instantiate IrtGeo-derived object, depending on detector
      auto rich = detector_name;
      std::transform(rich.begin(), rich.end(), rich.begin(), ::toupper);
      if     ( rich=="DRICH"  ) m_irtGeo = new IrtGeoDRICH(m_dd4hepGeo,  verbose);
      else if( rich=="PFRICH" ) m_irtGeo = new IrtGeoPFRICH(m_dd4hepGeo, verbose);
      else throw JException(fmt::format("IrtGeo is not defined for detector '{}'",detector_name));
    };
    std::call_once(init_flag, initialize);
  }
  catch (std::exception &ex) {
    throw JException(ex.what());
  }

  return m_irtGeo;
}

// Destructor --------------------------------------------------------
IrtGeo_service::~IrtGeo_service(){
  try {
    if(m_dd4hepGeo) m_dd4hepGeo->destroyInstance();
    m_dd4hepGeo = nullptr;
    if(m_irtGeo) delete m_irtGeo;
  } catch (...) {}
}
