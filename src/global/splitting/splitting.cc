// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include "HitChecker.h"

#include "TimeframeSplitter.h"

#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>

extern "C" {
void InitPlugin(JApplication* app) {

  InitJANAPlugin(app);

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());

  app->Add(new JOmniFactoryGeneratorT<HitChecker>({.tag          = "timeslice_hit_checker",
                                                   .level        = JEventLevel::Timeslice,
                                                   .input_names  = {"SiBarrelHits"},
                                                   .output_names = {"ts_checked_hits"}}));

  app->Add(new JOmniFactoryGeneratorT<HitChecker>({.tag          = "physics_hit_checker",
                                                   .level        = JEventLevel::PhysicsEvent,
                                                   .input_names  = {"SiBarrelHits"},
                                                   .output_names = {"phys_checked_hits"}}));

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
