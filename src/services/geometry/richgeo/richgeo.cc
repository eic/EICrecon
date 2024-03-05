// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, Christopher Dilks

#include <JANA/JApplication.h>
#include <memory>

#include "RichGeo_service.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->ProvideService(std::make_shared<RichGeo_service>(app) );
  }
}
