// Copyright 2024, EICrecon contributors
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JParameterManager.h>
#include <string>

#include "JEventProcessorJANADOT.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new JEventProcessorJANADOT());
  app->GetJParameterManager()->SetParameter("RECORD_CALL_STACK", true);
}
}