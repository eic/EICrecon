// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TMath.h>
#include <edm4eic/unit_system.h>
#include <cmath>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/EICROCDigitization_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconPulseDiscretization_factory.h"
#include "factories/digi/SiliconPulseGeneration_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "TOFEndcapRawHits", {"EventHeader", "TOFEndcapHits"},
      {"TOFEndcapRawHits", "TOFEndcapRawHitAssociations"},
      {
          .threshold      = 6.0 * dd4hep::keV,
          .timeResolution = 0.025,
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "TOFEndcapRecHits", {"TOFEndcapRawHits"}, // Input data collection tags
      {"TOFEndcapRecHits"},                     // Output data tag
      {
          .timeResolution = 0.025,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<SiliconChargeSharing_factory>(
      "TOFEndcapSharedHits", {"TOFEndcapHits"}, {"TOFEndcapSharedHits"},
      {
          .sigma_sharingx = 0.1 * dd4hep::cm,
          .sigma_sharingy = 0.1 * dd4hep::cm,
          .min_edep       = 0.0 * edm4eic::unit::GeV,
          .readout        = "TOFEndcapHits",
      },
      app));

  const double x_when_landau_min = -0.22278;
  const double landau_min        = TMath::Landau(x_when_landau_min, 0, 1, true);
  const double sigma_analog      = 0.293951 * edm4eic::unit::ns;
  const double Vm                = 3e-4 * dd4hep::GeV;
  const double adc_range         = 256;

  const double gain = -adc_range / Vm / landau_min * sigma_analog;
  const int offset  = 3;
  app->Add(new JOmniFactoryGeneratorT<SiliconPulseGeneration_factory>(
      "TOFEndcapSmoothPulses", {"TOFEndcapSharedHits"}, {"TOFEndcapSmoothPulses"},
      {
          .pulse_shape_function = "LandauPulse",
          .pulse_shape_params   = {gain, sigma_analog, offset},
          .ignore_thres         = 0.05 * adc_range,
          .timestep             = 0.01 * edm4eic::unit::ns,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      "TOFEndcapCombinedPulses", {"TOFEndcapSmoothPulses"}, {"TOFEndcapCombinedPulses"},
      {
          .minimum_separation = 25 * edm4eic::unit::ns,
      },
      app));

  double risetime = 0.45 * edm4eic::unit::ns;
  app->Add(new JOmniFactoryGeneratorT<SiliconPulseDiscretization_factory>(
      "SiliconPulseDiscretization", {"TOFEndcapCombinedPulses"}, {"TOFEndcapPulses"},
      {
          .EICROC_period = 25 * edm4eic::unit::ns,
          .local_period  = 25 * edm4eic::unit::ns / 1024,
          .global_offset = -offset * sigma_analog + risetime,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<EICROCDigitization_factory>(
      "EICROCDigitization", {"TOFEndcapPulses"}, {"TOFEndcapADCTDC"}, {}, app));
}
} // extern "C"
