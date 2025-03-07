// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <algorithm>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
#include "factories/digi/LGADChargeSharing_factory.h"
#include "factories/digi/LGADPulseGeneration_factory.h"
#include "factories/digi/LGADPulseDigitization_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "TOFBarrelRawHits",
      {
        "TOFBarrelHits"
      },
      {
        "TOFBarrelRawHits",
        "TOFBarrelRawHitAssociations"
      },
      {
        .threshold = 6.0 * dd4hep::keV,
        .timeResolution = 0.025,    // [ns]
      },
      app
  ));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "TOFBarrelRecHits",
      {"TOFBarrelRawHits"},    // Input data collection tags
      {"TOFBarrelRecHits"},    // Output data tag
      {
          .timeResolution = 10,
      },
      app
  ));         // Hit reco default config for factories




  app->Add(new JOmniFactoryGeneratorT<LGADChargeSharing_factory>(
      "LGADChargeSharing",
      {"TOFBarrelHits"},
      {"TOFBarrelSharedHits"},
      {
          .sigma_sharingx = 0.1 * dd4hep::cm,
          .sigma_sharingy = 0.5 * dd4hep::cm,
          .readout = "TOFBarrelHits",
          .same_sensor_condition = "sensor_1 == sensor_2",
          .neighbor_fields = {"x", "y"}
      },
      app
  ));

  app->Add(new JOmniFactoryGeneratorT<LGADPulseGeneration_factory>(
      "LGADPulseGeneration",
      {"TOFBarrelSharedHits"},
      {"TOFBarrelPulse"},
      {},
      app
  ));

  app->Add(new JOmniFactoryGeneratorT<LGADPulseDigitization_factory>(
      "LGADPulseDigitization",
      {"TOFBarrelPulse"},
      {"TOFBarrelADCTDC"},
      {},
      app
  ));

}
} // extern "C"
