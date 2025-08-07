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
void InitPlugin_digiECTRK(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>({
      .tag          = "SiEndcapTrackerRawHits_TK",
      .level        = JEventLevel::Timeslice,
      .input_names  = {"EventHeader", "TrackerEndcapHits"},
      .output_names = {"SiEndcapTrackerRawHits_TK", "SiEndcapTrackerRawHitAssociations_TK"},
      .configs =
          {
              .threshold = 0.54 * dd4hep::keV,
          },
  }));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "SiEndcapTrackerRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"SiEndcapTrackerRawHits_TK"},
       .output_names = {"SiEndcapTrackerRecHits_TK"},
       .configs      = {
                .timeResolution = 10,
       }}));
}
// } // extern "C"
