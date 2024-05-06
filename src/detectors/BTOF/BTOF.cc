// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
#include "factories/pid_lut/PIDLookup_factory.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
        "TOFBarrelRawHit",
        {
          "TOFBarrelHits"
        },
        {
          "TOFBarrelRawHit",
          "TOFBarrelHitAssociations"
        },
        {
            .threshold = 6.0 * dd4hep::keV,
            .timeResolution = 0.025,    // [ns]
        },
        app
    ));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
        "TOFBarrelRecHit",
        {"TOFBarrelRawHit"},    // Input data collection tags
        {"TOFBarrelRecHit"},     // Output data tag
        {
            .timeResolution = 10,
        },
        app
    ));         // Hit reco default config for factories

    int BarrelTOF_ID = 0;
    try {
        auto detector = app->GetService<DD4hep_service>()->detector();
        BarrelTOF_ID = detector->constant<int>("BarrelTOF_ID");
    } catch(const std::runtime_error&) {
        // Nothing
    }
    app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
          "TOFPID",
          {"ReconstructedChargedWithoutPIDParticles", "ReconstructedChargedWithoutPIDParticleAssociations"},
          {"TOFPID", "TOFParticleIDs"},
          {
            .filename="calibrations/tof.lut",
            .system=BarrelTOF_ID,
            .pdg_values={11, 211, 321, 2212},
            .charge_values={1},
            .momentum_edges={0.0, 0.3, 0.6, 0.9, 1.2, 1.5, 1.8, 2.1, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.2, 4.5, 4.8, 5.1, 5.4, 5.7, 6.0},
            .polar_edges={2.50, 10.95, 19.40, 27.85, 36.30, 44.75, 53.20, 61.65, 70.10, 78.55, 87.00, 95.45, 103.90, 112.35, 120.80, 129.25, 137.70, 146.15, 154.60},
            .azimuthal_binning={0., 360., 360.}, // lower, upper, step
            .polar_bin_centers_in_lut=true,
          },
          app
          ));

}
} // extern "C"
