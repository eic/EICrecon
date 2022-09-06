// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

#include "ACTSGeo_service.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <DD4hep/Printout.h>
#include <JANA/JException.h>

//----------------------------------------------------------------
// detector
//
/// Return pointer to the dd4hep::Detector object.
/// Call Initialize if needed.
//----------------------------------------------------------------
std::shared_ptr<const GeoSvc> ACTSGeo_service::acts_context() {
    std::call_once( init_flag, &ACTSGeo_service::Initialize, this);
    return m_acts_context;
}

void ACTSGeo_service::Initialize() {
    if(!m_dd4hepGeo) {
        throw JException("ACTSGeo_service m_dd4hepGeo==null which should never be!");
    }

    m_acts_context = std::make_shared<GeoSvc>();
    m_acts_context->initialize(m_dd4hepGeo);
}

void ACTSGeo_service::acquire_services(JServiceLocator * srv_locator) {

    auto dd4hep_service = srv_locator->get<JDD4hep_service>();
    m_dd4hepGeo = dd4hep_service->detector();

}
