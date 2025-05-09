// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include "TimeframeSplitter.h"
#include <JANA/JApplication.h>

extern "C" {
void InitPlugin(JApplication* app) {

  InitJANAPlugin(app);

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());

  // Factory that produces timeslice-level protoclusters from timeslice-level hits
  /*
    app->Add(new JOmniFactoryGeneratorT<MyProtoclusterFactory>(
                { .tag = "timeslice_protoclusterizer",
                  .level = JEventLevel::Timeslice,
                  .input_names = {"hits"},
                  .output_names = {"ts_protoclusters"}
                }));
    */
}
} // "C"
