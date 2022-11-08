// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeo_service.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->ProvideService(std::make_shared<IrtGeo_service>(app) );
  }
}
