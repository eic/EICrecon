// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 ePIC Collaboration

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/AlignmentDerivativeSetCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/tracking/MeasurementToMille_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Register the MeasurementToMille factory.
  //
  // Input collections:
  //   - "CentralCKFTracks"          (edm4eic::TrackCollection)
  //   - "CentralTrackerMeasurements" (edm4eic::Measurement2DCollection)
  //
  // Output collection:
  //   - "SiliconAlignmentDerivatives" (edm4eic::AlignmentDerivativeSetCollection)
  //
  // NOTE: The EDM4eic AlignmentDerivativeSet type used here is defined in the
  // ePIC alignment branch of EDM4eic and is not yet part of an official EDM4eic
  // release.  Update the EDM4eic dependency to the alignment-enabled build before
  // compiling this plugin.
  app->Add(new JOmniFactoryGeneratorT<MeasurementToMille_factory>(
      "SiliconAlignmentDerivatives",
      {
          "CentralCKFTracks",
          "CentralTrackerMeasurements",
      },
      {
          "SiliconAlignmentDerivatives",
      },
      {
          .maxChi2PerNDF = 5.0f,
          .minMomentum   = 1.0f,
          .fixedLayers   = {0}, // fix SagittaSiBarrel_layer1 as reference by default
      },
      app));
}
} // extern "C"
