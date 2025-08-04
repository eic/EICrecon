// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// extern "C" {
void InitPlugin_digiB0TRK(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .m_tag                 = "B0TrackerRawHits_TK",
          .m_default_input_tags  = {"B0TrackerHits"},
          .m_default_output_tags = {"B0TrackerRawHits_TK", "B0TrackerRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .threshold      = 10.0 * dd4hep::keV,
                  .timeResolution = 8,
              },
          .level = JEventLevel::Timeslice},
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "B0TrackerRecHits_TK",
          .m_default_input_tags  = {"B0TrackerRawHits_TK"},
          .m_default_output_tags = {"B0TrackerRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 8,
              },
          .level = JEventLevel::Timeslice},
      app));
}
// } // extern "C"
