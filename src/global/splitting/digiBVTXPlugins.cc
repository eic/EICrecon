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
void InitPlugin_digiBVTX(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
            .m_tag           = "SiBarrelVertexRawHits_TK",
            .m_default_input_tags   = {"VertexBarrelHits"},
            .m_default_output_tags  = {"SiBarrelVertexRawHits_TK", "SiBarrelVertexRawHitAssociations_TK"},
            .m_default_cfg = {
                .threshold = 0.54 * dd4hep::keV,
                .timeResolution = 10,
            },
            .level         = JEventLevel::Timeslice
        },
        app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag           = "SiBarrelVertexRecHits_TK",
          .m_default_input_tags   = {"SiBarrelVertexRawHits_TK"},
          .m_default_output_tags  = {"SiBarrelVertexRecHits_TK"},
          .m_default_cfg = {
              .timeResolution = 10,
          },
          .level         = JEventLevel::Timeslice
      },
      app));
}
// } // extern "C"
