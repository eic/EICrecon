// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>

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
    SiliconTrackerDigiConfig barrel_digi_default_cfg;
    barrel_digi_default_cfg.threshold = 0 * dd4hep::keV;
    barrel_digi_default_cfg.timeResolution = 8;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"MPGDBarrelHits"}, "MPGDBarrelDigiHits", barrel_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig tracker_hit_reco_cfg;
    tracker_hit_reco_cfg.time_resolution = 8;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"MPGDBarrelDigiHits"},     // Input data collection tags
            "MPGDBarrelRecHits",    // Output data tag
             tracker_hit_reco_cfg));           // Hit reco default config for factories

    // Digitization
    SiliconTrackerDigiConfig dirc_digi_default_cfg;
    barrel_digi_default_cfg.threshold = 0 * dd4hep::keV;
    barrel_digi_default_cfg.timeResolution = 8;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"MPGDDIRCHits"}, "MPGDDIRCDigiHits", dirc_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig dirc_hit_reco_cfg;
    tracker_hit_reco_cfg.time_resolution = 8;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"MPGDDIRCDigiHits"},     // Input data collection tags
            "MPGDDIRCRecHits",   // Output data tag
            dirc_hit_reco_cfg));          // Hit reco default config for factories

}
} // extern "C"
