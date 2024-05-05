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
          {"ReconstructedParticles", "ReconstructedParticleAssociations"},
          {"TOFPID", "TOFParticleIDs"},
          {
            .filename="calibrations/tof.lut",
            .system=BarrelTOF_ID,
            .pdg_values={11, 211, 321, 2212},
            .charge_values={1},
            .momentum_edges={0.15, 0.45, 0.75, 1.05, 1.35, 1.65, 1.95, 2.25, 2.55, 2.85, 3.15, 3.45, 3.75, 4.05, 4.35, 4.65, 4.95, 5.25, 5.55, 5.85, 6.15},
            .polar_edges={6.72, 15.17, 23.62, 32.07, 40.52, 48.97, 57.42, 65.88, 74.32, 82.78, 91.22, 99.67, 108.12, 116.57, 125.02, 133.47, 141.92, 150.38},
            .azimuthal_binning={0., 360., 360.}, // lower, upper, step
          },
          app
          ));

}
} // extern "C"
