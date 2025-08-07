// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>

#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// extern "C" {
void InitPlugin_digiB0TRK(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      {.tag          = "B0TrackerRawHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"EventHeader", "B0TrackerHits"},
       .output_names = {"B0TrackerRawHits_TK", "B0TrackerRawHitAssociations_TK"},
       .configs      = {
                .threshold      = 10.0 * dd4hep::keV,
                .timeResolution = 8,
       }}));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "B0TrackerRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"B0TrackerRawHits_TK"},
       .output_names = {"B0TrackerRecHits_TK"},
       .configs      = {
                .timeResolution = 8,
       }}));
}
// } // extern "C"
