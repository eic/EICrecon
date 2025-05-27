// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <JANA/JApplicationFwd.h>
#include <algorithms/service.h>

#include "PIDLookupTableSvc.h"

extern "C" {

void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  auto& serviceSvc        = algorithms::ServiceSvc::instance();
  auto& pidLookupTableSvc = eicrecon::PIDLookupTableSvc::instance();
  serviceSvc.add<eicrecon::PIDLookupTableSvc>(&pidLookupTableSvc);
}
}
