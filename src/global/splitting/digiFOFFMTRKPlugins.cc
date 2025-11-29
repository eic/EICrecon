// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <vector>

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// extern "C" {
void InitPlugin_digiFOFFMTRK(JApplication* app) {
  InitJANAPlugin(app);
  using namespace eicrecon;
  //Digitized hits, especially for thresholds
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .m_tag                 = "ForwardOffMTrackerRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "ForwardOffMTrackerHits"},
          .m_default_output_tags = {"ForwardOffMTrackerRawHits_TK",
                                    "ForwardOffMTrackerRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .threshold      = 10.0 * dd4hep::keV,
                  .timeResolution = 8,
              },
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>::TypedWiring{
          .m_tag                 = "ForwardOffMTrackerRecHits_TK",
          .m_default_input_tags  = {"ForwardOffMTrackerRawHits_TK"},
          .m_default_output_tags = {"ForwardOffMTrackerRecHits_TK"},
          .m_default_cfg =
              {
                  .timeResolution = 8,
              },
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<MatrixTransferStatic_factory>(
      JOmniFactoryGeneratorT<MatrixTransferStatic_factory>::TypedWiring{
          .m_tag                 = "ForwardOffMRecParticles_TK",
          .m_default_input_tags  = {"MCParticles", "ForwardOffMTrackerRecHits_TK"},
          .m_default_output_tags = {"ForwardOffMRecParticles_TK"},
          .m_default_cfg =
              {
                  .matrix_configs = {{
                      .nomMomentum = 130.0,

                      .aX =
                          {
                              {1.61591, 12.6786},
                              {0.184206, -2.907},
                          },

                      .aY =
                          {
                              {-0.789385, -28.5578},
                              {-0.0721796, -2.8763},
                          },

                      .local_x_offset       = -881.631,
                      .local_y_offset       = -0.00552173,
                      .local_x_slope_offset = -59.7386,
                      .local_y_slope_offset = -0.00360656,

                  }},

                  .hit1minZ = 22490.0,
                  .hit1maxZ = 22512.0,
                  .hit2minZ = 24512.0,
                  .hit2maxZ = 24535.0,

                  .readout = "ForwardOffMTrackerRecHits_TK",
              },
          .level = JEventLevel::Timeslice},
      app));
}
// }
