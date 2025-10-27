// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <memory>

#include "DD4hep_service.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->ProvideService(std::make_shared<DD4hep_service>(app));
}
}
