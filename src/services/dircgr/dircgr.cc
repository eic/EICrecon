// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include <JANA/JApplicationFwd.h>
#include <algorithms/service.h>

#include "DircGRCalibSvc.h"

extern "C" {

void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  auto& serviceSvc = algorithms::ServiceSvc::instance();
  auto& dircgrSvc  = eicrecon::DircGRCalibSvc::instance();
  serviceSvc.add<eicrecon::DircGRCalibSvc>(&dircgrSvc);
}
}
