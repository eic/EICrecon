// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JParameterManager.h>
#include <memory>

#include "JEventProcessorJANATOP.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new JEventProcessorJANATOP());
  app->GetJParameterManager()->SetParameter("RECORD_CALL_STACK", true);
}
}
