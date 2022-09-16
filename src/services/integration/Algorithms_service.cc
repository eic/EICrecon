// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "Algorithms_service.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>


void eicrecon::Algorithms_service::acquire_services(JServiceLocator *srv_locator) {
    auto dd4hep_service = srv_locator->get<JDD4hep_service>();
}
