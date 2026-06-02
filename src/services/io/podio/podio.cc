// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <JANA/Services/JParameterManager.h>
#include <string>

#include "JEventProcessorManagedPODIO.h"
#include "JEventProcessorPODIO.h"
#include "JEventSourceManagedPODIO.h"
#include "JEventSourcePODIO.h"

// Make this a JANA plugin
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  // Check if managed mode is requested
  if (app->GetJParameterManager()->Exists("podio:managed_socket_path")) {
    app->Add(new JEventSourceManagedPODIO());
    app->Add(new JEventProcessorManagedPODIO());
  } else {
    app->Add(new JEventSourceGeneratorT<JEventSourcePODIO>());
    app->Add(new JEventProcessorPODIO());
  }
}
}
