// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

// input is the photoelectrons, and tracking info
// output is Cherenkov PID
// - prepares for and calls the IRT (standalone) algorithm

#pragma once

#include <cmath>

// JANA
#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>

// data model
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/CherenkovParticleID.h>

// services
#include <services/geometry/irt/IrtGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CherenkovParticleID_factory_IrtParticleID : public JFactoryT<edm4eic::CherenkovParticleID> {
  public:

    //------------------------------------------
    CherenkovParticleID_factory_IrtParticleID() {
      SetTag("IrtParticleID"); // FIXME: should be D/PFRICHIrtParticleID ?
    }

    //------------------------------------------
    void Init() override{
      auto app = GetApplication();

      // default params
      m_detector_name = "DRICH"; // FIXME: respect https://github.com/eic/EICrecon/pull/242
      auto tag = "RICH:"+GetTag();
      app->SetDefaultParameter(tag+":which_rich", m_detector_name, "Indicate which RICH to use");

      // services
      m_irtGeoSvc = app->template GetService<IrtGeo_service>();
      m_irtGeo = m_irtGeoSvc->IrtGeometry(m_detector_name);
      m_log = app->GetService<Log_service>()->logger(GetTag());

      // set log level
      std::string log_level_str = "info";
      auto pm = app->GetJParameterManager();
      pm->SetDefaultParameter(tag + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
      m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

      m_log->info("\n\nUSING RICH: {}\n\n",m_detector_name);

    }

    //------------------------------------------
    void Process(const std::shared_ptr<const JEvent> &event) override {

      // inputs
      // FIXME: will be changed to photoelectrons; until then, just use all the photon hits
      auto photoelectrons = event->Get<edm4hep::SimTrackerHit>(m_detector_name+"Hits");

      // loop over photoelectrons
      // FIXME: at the moment, we do nothing; the current version of this factory is only meant to test the `irt` service
      std::vector<edm4eic::CherenkovParticleID*> output_pid;
      // for( const auto& photoelectron : photoelectrons ) {
      //   auto pid = new edm4eic::CherenkovParticleID(
      //       ...
      //       );
      //   output_pid.push_back(pid);
      // }

      // outputs
      Set(output_pid);
    }

  private:
    std::string m_detector_name;
    std::shared_ptr<IrtGeo_service> m_irtGeoSvc;
    std::shared_ptr<spdlog::logger> m_log;
    CherenkovDetectorCollection *m_irtGeo;

};
