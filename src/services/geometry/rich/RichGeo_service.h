// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#pragma once

#include <cstdlib>
#include <algorithm>

// JANA
#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>
#include <JANA/JException.h>

// services
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

// richgeo
#include "RichGeo.h"
#include "IrtGeo.h"
#include "IrtGeoDRICH.h"
#include "IrtGeoPFRICH.h"
#include "ActsGeo.h"

class RichGeo_service : public JService {
  public:
    RichGeo_service(JApplication *app) : m_app(app) {}
    ~RichGeo_service();

    // return pointers to geometry bindings; initializes the bindings upon the first time called
    rich::IrtGeo *GetIrtGeo(std::string detector_name);
    rich::ActsGeo *GetActsGeo(std::string detector_name);

  private:
    RichGeo_service() = default;
    void acquire_services(JServiceLocator *) override;

    std::once_flag   m_init_irt;
    std::once_flag   m_init_acts;
    JApplication     *m_app       = nullptr;
    dd4hep::Detector *m_dd4hepGeo = nullptr;
    rich::IrtGeo     *m_irtGeo    = nullptr;
    rich::ActsGeo    *m_actsGeo   = nullptr;

    std::shared_ptr<spdlog::logger> m_log;
    bool m_verbose;
};
