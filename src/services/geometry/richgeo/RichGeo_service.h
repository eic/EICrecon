// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <cstdlib>
#include <algorithm>

// JANA
#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>
#include <JANA/JException.h>

// services
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

// richgeo
#include "RichGeo.h"
#include "IrtGeo.h"
#include "IrtGeoDRICH.h"
#include "IrtGeoPFRICH.h"
#include "ActsGeo.h"
#include "ReadoutGeo.h"

class RichGeo_service : public JService {
  public:
    RichGeo_service(JApplication *app) : m_app(app) {}
    virtual ~RichGeo_service();

    // return pointer to the main DD4hep Detector
    virtual dd4hep::Detector *GetDD4hepGeo() { return m_dd4hepGeo; };

    // return pointers to geometry bindings; initializes the bindings upon the first time called
    virtual richgeo::IrtGeo *GetIrtGeo(std::string detector_name);
    virtual richgeo::ActsGeo *GetActsGeo(std::string detector_name);
    virtual std::shared_ptr<richgeo::ReadoutGeo> GetReadoutGeo(std::string detector_name);

  private:
    RichGeo_service() = default;
    void acquire_services(JServiceLocator *) override;

    std::once_flag   m_init_irt;
    std::once_flag   m_init_acts;
    std::once_flag   m_init_readout;
    JApplication        *m_app        = nullptr;
    dd4hep::Detector    *m_dd4hepGeo  = nullptr;
    richgeo::IrtGeo     *m_irtGeo     = nullptr;
    richgeo::ActsGeo    *m_actsGeo    = nullptr;
    std::shared_ptr<richgeo::ReadoutGeo> m_readoutGeo;

    std::shared_ptr<spdlog::logger> m_log;
};
