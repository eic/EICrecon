// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <JANA/JApplicationFwd.h>
#include <algorithms/service.h>

#include "EvaluatorSvc.h"

extern "C" {

void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  auto& serviceSvc   = algorithms::ServiceSvc::instance();
  auto& evaluatorSvc = eicrecon::EvaluatorSvc::instance();
  serviceSvc.add<eicrecon::EvaluatorSvc>(&evaluatorSvc);
}
}
