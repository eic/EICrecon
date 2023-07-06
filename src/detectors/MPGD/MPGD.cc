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
    barrel_digi_default_cfg.threshold = 0.25 * dd4hep::keV;
    barrel_digi_default_cfg.timeResolution = 10;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"MPGDBarrelHits"}, "MPGDBarrelRawHits", barrel_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig tracker_hit_reco_cfg;
    tracker_hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"MPGDBarrelRawHits"},     // Input data collection tags
            "MPGDBarrelRecHits",    // Output data tag
             tracker_hit_reco_cfg));           // Hit reco default config for factories

    // Digitization
    SiliconTrackerDigiConfig dirc_digi_default_cfg;
    dirc_digi_default_cfg.threshold = 0.25 * dd4hep::keV;
    dirc_digi_default_cfg.timeResolution = 10;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"MPGDDIRCHits"}, "MPGDDIRCRawHits", dirc_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig dirc_hit_reco_cfg;
    dirc_hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"MPGDDIRCRawHits"},     // Input data collection tags
            "MPGDDIRCRecHits",   // Output data tag
            dirc_hit_reco_cfg));          // Hit reco default config for factories

    // Digitization
    SiliconTrackerDigiConfig outerMPGDbarrel_digi_default_cfg;
   outerMPGDbarrel_digi_default_cfg.threshold = 0.25 * dd4hep::keV;
   outerMPGDbarrel_digi_default_cfg.timeResolution = 10;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"OuterMPGDBarrelHits"}, "OuterMPGDBarrelRawHits", outerMPGDbarrel_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig outerMPGDbarrel_hit_reco_cfg;
    outerMPGDbarrel_hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"OuterMPGDBarrelRawHits"},     // Input data collection tags
            "OuterMPGDBarrelRecHits",   // Output data tag
            outerMPGDbarrel_hit_reco_cfg));          // Hit reco default config for factories


    // Digitization
    SiliconTrackerDigiConfig backwardMPGDendcap_digi_default_cfg;
    backwardMPGDendcap_digi_default_cfg.threshold = 0.25 * dd4hep::keV;
    backwardMPGDendcap_digi_default_cfg.timeResolution = 10;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"BackwardMPGDEndcapHits"}, "BackwardMPGDEndcapRawHits", backwardMPGDendcap_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig backwardMPGDendcap_hit_reco_cfg;
    backwardMPGDendcap_hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"BackwardMPGDEndcapRawHits"},     // Input data collection tags
            "BackwardMPGDEndcapRecHits",   // Output data tag
            backwardMPGDendcap_hit_reco_cfg));          // Hit reco default config for factories

    // Digitization
    SiliconTrackerDigiConfig forwardMPGDendcap_digi_default_cfg;
    forwardMPGDendcap_digi_default_cfg.threshold = 0.25 * dd4hep::keV;
    forwardMPGDendcap_digi_default_cfg.timeResolution = 10;
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"ForwardMPGDEndcapHits"}, "ForwardMPGDEndcapRawHits", forwardMPGDendcap_digi_default_cfg));

    // Convert raw digitized hits into hits with geometry info (ready for tracking)
    TrackerHitReconstructionConfig forwardMPGDendcap_hit_reco_cfg;
    forwardMPGDendcap_hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>(
            {"ForwardMPGDEndcapRawHits"},     // Input data collection tags
            "ForwardMPGDEndcapRecHits",   // Output data tag
            forwardMPGDendcap_hit_reco_cfg));          // Hit reco default config for factories
}
} // extern "C"
