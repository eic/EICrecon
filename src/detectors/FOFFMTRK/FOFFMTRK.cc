// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  using namespace eicrecon;
  using jana::components::JOmniFactoryGeneratorT;

  //Digitized hits, especially for thresholds
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "ForwardOffMTrackerRawHits", {"EventHeader", "ForwardOffMTrackerHits"},
      {"ForwardOffMTrackerRawHits", "ForwardOffMTrackerRawHitAssociations"},
      {
          .threshold      = 10.0 * dd4hep::keV,
          .timeResolution = 8,
      }));

  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "ForwardOffMTrackerRecHits", {"ForwardOffMTrackerRawHits"}, {"ForwardOffMTrackerRecHits"},
      {
          .timeResolution = 8,
      }));

  app->Add(new JOmniFactoryGeneratorT<MatrixTransferStatic_factory>(
      "ForwardOffMRecParticles", {"MCParticles", "ForwardOffMTrackerRecHits"},
      {"ForwardOffMRecParticles"},
      {
          .matrix_configs = {{
              .nomMomentum = 130.0,

              .aX =
                  {
                      {2.08344, 5.37571},
                      {0.188756, -2.90941},
                  },

              .aY =
                  {
                      {-0.977013, -35.7785},
                      {-0.0812252, -2.86315},
                  },

              .local_x_offset       = -1032.2,
              .local_y_offset       = 0.00462829,
              .local_x_slope_offset = -59.7363,
              .local_y_slope_offset = -0.0030213,

          }},

          .hit1minZ = 25490.0,
          .hit1maxZ = 25512.0,
          .hit2minZ = 27012.0,
          .hit2maxZ = 27035.0,

          .readout = "ForwardOffMTrackerRecHits",
      }));
}
}
