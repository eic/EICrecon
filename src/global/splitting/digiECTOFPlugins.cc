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
void InitPlugin_digiECTOF(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      {.tag          = "TOFEndcapRawHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"EventHeader", "TOFEndcapHits"},
       .output_names = {"TOFEndcapRawHits_TK", "TOFEndcapRawHitAssociations_TK"},
       .configs      = {
                .threshold      = 6.0 * dd4hep::keV,
                .timeResolution = 0.025,
       }}));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "TOFEndcapRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"TOFEndcapRawHits_TK"},
       .output_names = {"TOFEndcapRecHits_TK"},
       .configs      = {
                .timeResolution = 0.025,
       }}));
}
// } // extern "C"
