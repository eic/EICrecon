// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "JANA/Components/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// extern "C" {
void InitPlugin_digiBTRK(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>({
      .tag          = "SiBarrelRawHits_TK",
      .level        = JEventLevel::Timeslice,
      .input_names  = {"EventHeader", "SiBarrelHits"},
      .output_names = {"SiBarrelRawHits_TK", "SiBarrelRawHitAssociations_TK"},
      .configs =
          {
              .threshold = 0.54 * dd4hep::keV,
          },
  }));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "SiBarrelTrackerRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"SiBarrelRawHits_TK"},
       .output_names = {"SiBarrelTrackerRecHits_TK"},
       .configs      = {
                .timeResolution = 10,
       }}));
}
// } // extern "C"
