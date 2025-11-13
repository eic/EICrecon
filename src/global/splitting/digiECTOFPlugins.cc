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
void InitPlugin_digiECTOF(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .m_tag                 = "TOFEndcapRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "TOFEndcapHits"},
          .m_default_output_tags = {"TOFEndcapRawHits_TK", "TOFEndcapRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .threshold      = 6.0 * dd4hep::keV,
                  .timeResolution = 0.025,
              },
          .level = JEventLevel::Timeslice},
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "TOFEndcapRecHits_TK",
          .m_default_input_tags  = {"TOFEndcapRawHits_TK"},
          .m_default_output_tags = {"TOFEndcapRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 0.025,
              },
          .level = JEventLevel::Timeslice},
      app));
}
// } // extern "C"
