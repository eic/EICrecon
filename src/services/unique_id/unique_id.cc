// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <memory>

#include "services/unique_id/UniqueIDGen_service.h"

extern "C" {

void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->ProvideService(std::make_shared<UniqueIDGen_service>(app));
}
}
