// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/unit_system.h>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Luminosity pair-spectrometer tracker (epic compact/far_backward/lumi/spec_tracker.xml):
  // two planes (module 0, 1) of top and bottom silicon sectors in front of EcalLumiSpec,
  // 0.3 mm thick, segmented as CartesianGridXY with LumiSpecTracker_pixelSize = 0.1 mm.
  // The pixel size comes from the geometry; there is no sensor-specific readout model yet.
  // - threshold: as for the other far-region silicon trackers (RPOTS, FOFFMTRK), well below
  //   the most probable MIP deposit of about 80 keV in 0.3 mm of silicon.
  // - timeResolution: AC-LGAD value used by the far-forward trackers (B0TRK, RPOTS, FOFFMTRK).
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "LumiSpecTrackerRawHits", {"EventHeader", "LumiSpecTrackerHits"},
      {"LumiSpecTrackerRawHits", "LumiSpecTrackerRawHitLinks", "LumiSpecTrackerRawHitAssociations"},
      {
          .threshold      = 10.0 * dd4hep::keV,
          .timeResolution = 30 * edm4eic::unit::ps,
      },
      app));
}
}
