// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplicationFwd.h>

#include "TrackPropagationTest_processor.h"

extern "C" {
void InitPlugin(JApplication* app) {

  // Initializes this plugin
  InitJANAPlugin(app);

  // Adds our processor to JANA2 to execute
  app->Add(new TrackPropagationTest_processor());
}
}
