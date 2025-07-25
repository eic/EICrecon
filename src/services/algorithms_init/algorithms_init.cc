// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include <JANA/JApplication.h>
#include <memory>

#include "AlgorithmsInit_service.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->ProvideService(std::make_shared<AlgorithmsInit_service>(app));
}
}
