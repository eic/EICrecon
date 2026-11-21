// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, EICrecon contributors

#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JParameterManager.h>
#include <string>

#include "JEventProcessorCallGraph.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new JEventProcessorCallGraph());
  app->GetJParameterManager()->SetParameter("RECORD_CALL_STACK", true);
}
}
