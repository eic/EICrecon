// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/digi/SiliconTrackerDigi_factory.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>

#include <algorithms/digi/SiliconTrackerDigiConfig.h>
#include <algorithms/tracking/TrackerHitReconstructionConfig.h>


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    SiliconTrackerDigiConfig digi_default_cfg;
    digi_default_cfg.threshold = 0 * dd4hep::keV;
    digi_default_cfg.timeResolution = 8;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"B0TrackerHits"}, "B0TrackerHitRawHit", digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig hit_reco_cfg;
    hit_reco_cfg.time_resolution = 8;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"B0TrackerHitRawHit"},     // Input data collection tags
            "B0TrackerRecHits",         // Output data tag
             hit_reco_cfg));            // Hit reco default config for factories

}
} // extern "C"
