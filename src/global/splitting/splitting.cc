// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <extensions/jana/JOmniUnfolderGeneratorT.h>
#include <string>

#include "TimeframeSplitter.h"

extern "C" {
void InitPlugin(JApplication* app) {

  InitJANAPlugin(app);

  const bool splitTimeframes =
      app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting");
  if (!splitTimeframes) {
    return;
  }

  // Unfolder that takes timeframes and splits them into physics events.
  eicrecon::JOmniUnfolderGeneratorT<TimeframeSplitter> splitter_generator(
      {.tag          = "TimeframeSplitter",
       .parent_level = JEventLevel::Timeslice,
       .child_level  = JEventLevel::PhysicsEvent,
       //.input_names = {"EventHeader", "MCParticles"},
       //.variadic_input_names = {{}},
       //.output_names = {},
       //.variadic_output_names = {{}},
       .configs = {}});
  splitter_generator.Generate(app);
}
} // "C"
