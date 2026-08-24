// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Takuya Kumaoka

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/MutableCalorimeterHit.h>
#include <edm4eic/MutableTrackerHit.h>
#include <edm4eic/TrackerHit.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "TimeframeSplitter.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"

extern "C" {
void InitPlugin(JApplication* app) {

  InitJANAPlugin(app);

  const bool splitTimeframes =
      app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting");
  if (!splitTimeframes) {
    return;
  }

  // Unfolder that takes timeframes and splits them into physics events.
  app->Add(new TimeframeSplitter());
}
} // "C"
