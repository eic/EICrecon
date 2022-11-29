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

// IrtGeo
#include "irtgeo/IrtGeo.h"
#include "irtgeo/IrtGeoDRICH.h"
#include "irtgeo/IrtGeoPFRICH.h"

class IrtGeo_service : public JService {
  public:
    IrtGeo_service(JApplication *app) : m_app(app) {}
    ~IrtGeo_service();

    // return pointer to the IrtGeo, given a detector
    IrtGeo *GetIrtGeo(std::string detector_name);

  private:
    IrtGeo_service() = default;
    void acquire_services(JServiceLocator *) override;

    std::once_flag   init_flag;
    JApplication     *m_app       = nullptr;
    dd4hep::Detector *m_dd4hepGeo = nullptr;
    IrtGeo           *m_irtGeo    = nullptr;

    std::shared_ptr<spdlog::logger> m_log;
};
