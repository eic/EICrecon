// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>

#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"
#include "algorithms/fardetectors/PolynomialMatrixReconstructionConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/MatrixTransferStatic_factory.h"
#include "factories/fardetectors/PolynomialMatrixReconstruction_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using eicrecon::JOmniFactoryGeneratorT;

  MatrixTransferStaticConfig recon_cfg;
  PolynomialMatrixReconstructionConfig recon_poly_cfg;

  //Digitized hits, especially for thresholds
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "ForwardRomanPotRawHits", {"EventHeader", "ForwardRomanPotHits"},
      {"ForwardRomanPotRawHits", "ForwardRomanPotRawHitAssociations"},
      {
          .threshold      = 10.0 * dd4hep::keV,
          .timeResolution = 8,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "ForwardRomanPotRecHits", {"ForwardRomanPotRawHits"}, {"ForwardRomanPotRecHits"},
      {
          .timeResolution = 8,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<MatrixTransferStatic_factory>(
      "ForwardRomanPotStaticRecParticles",
      {
          "MCParticles",
          "ForwardRomanPotRecHits",
      },
      {
          "ForwardRomanPotStaticRecParticles",
      },
      {
          .matrix_configs =
              {{
                   .nomMomentum = 275.0,
                   .aX =
                       {
                           {3.251116, 30.285734},
                           {0.186036375, 0.196439472},
                       },
                   .aY =
                       {
                           {0.4730500000, 3.062999454},
                           {0.0204108951, -0.139318692},
                       },

                   .local_x_offset = -1209.29,   //-0.339334, these are the local coordinate values
                   .local_y_offset = 0.00132511, //-0.000299454,
                   .local_x_slope_offset = -45.4772,    //-0.219603248,
                   .local_y_slope_offset = 0.000745498, //-0.000176128,

               },
               {
                   // NOT TUNED -- just for testing purposes
                   .nomMomentum = 130.0,
                   .aX =
                       {
                           {3.16912, 22.4693},
                           {0.182402, -0.218209},
                       },

                   .aY =
                       {
                           {0.520743, 3.17339},
                           {0.0222482, -0.0923779},
                       },

                   .local_x_offset = -1209.29,   //-0.339334, these are the local coordinate values
                   .local_y_offset = 0.00132511, //-0.000299454,
                   .local_x_slope_offset = -45.4772,    //-0.219603248,
                   .local_y_slope_offset = 0.000745498, //-0.000176128,

               },
               {
                   .nomMomentum = 100.0,

                   .aX =
                       {
                           {3.152158, 20.852072},
                           {0.181649517, -0.303998487},
                       },

                   .aY =
                       {
                           {0.5306100000, 3.19623343},
                           {0.0226283320, -0.082666019},
                       },

                   .local_x_offset       = -1209.27,   //-0.329072,
                   .local_y_offset       = 0.00355218, //-0.00028343,
                   .local_x_slope_offset = -45.4737,   //-0.218525084,
                   .local_y_slope_offset = 0.00204394, //-0.00015321,

               },
               {
                   .nomMomentum = 41.0,

                   .aX =
                       {
                           {3.135997, 18.482273},
                           {0.176479921, -0.497839483},

                       },
                   .aY = {{0.4914400000, 4.53857451}, {0.0179664765, 0.004160679}},

                   .local_x_offset       = -1209.22,   //-0.283273,
                   .local_y_offset       = 0.00868737, //-0.00552451,
                   .local_x_slope_offset = -45.4641,   //-0.21174031,
                   .local_y_slope_offset = 0.00498786, //-0.003212011,

               }},
          .hit1minZ = 32541.0,
          .hit1maxZ = 32554.0,
          .hit2minZ = 34239.0,
          .hit2maxZ = 34252.0,

          .readout = "ForwardRomanPotRecHits",
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<PolynomialMatrixReconstruction_factory>(
      "ForwardRomanPotRecParticles",
      {
          "MCParticles",
          "ForwardRomanPotRecHits",
      },
      {
          "ForwardRomanPotRecParticles",
      },
      {
          .poly_matrix_configs = {{
                                      .nomMomentum = 275.0,
                                  },
                                  {
                                      .nomMomentum = 130.0,
                                  },
                                  {
                                      .nomMomentum = 100.0,
                                  },
                                  {
                                      .nomMomentum = 41.0,

                                  }},
          .hit1minZ            = 32541.0,
          .hit1maxZ            = 32554.0,
          .hit2minZ            = 34239.0,
          .hit2maxZ            = 34252.0,

          .readout = "ForwardRomanPotRecHits",
      },
      app));
}
}
