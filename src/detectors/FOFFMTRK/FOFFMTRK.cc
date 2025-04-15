// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <vector>

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  using namespace eicrecon;

  //Digitized hits, especially for thresholds
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "ForwardOffMTrackerRawHits", {"ForwardOffMTrackerHits"},
      {"ForwardOffMTrackerRawHits", "ForwardOffMTrackerRawHitAssociations"},
      {
          .threshold      = 10.0 * dd4hep::keV,
          .timeResolution = 8,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "ForwardOffMTrackerRecHits", {"ForwardOffMTrackerRawHits"}, {"ForwardOffMTrackerRecHits"},
      {
          .timeResolution = 8,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<MatrixTransferStatic_factory>(
      "ForwardOffMRecParticles", {"MCParticles", "ForwardOffMTrackerRecHits"},
      {"ForwardOffMRecParticles"},
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
          .hit1minZ       = 22499.0,
          .hit1maxZ       = 22522.0,
          .hit2minZ       = 24499.0,
          .hit2maxZ       = 24522.0,

          .readout = "ForwardOffMTrackerRecHits",
      },
      app));
}
}
