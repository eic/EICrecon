// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/digi/SiliconTrackerDigi_factory.h>
#include <global/tracking/TrackerHitReconstruction_factory.h>

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;


    // Digitization
    SiliconTrackerDigiConfig digi_cfg;
    
    app->Add(new JChainFactoryGeneratorT<SiliconTrackerDigi_factory>({"TaggerTrackerHits"}, "TaggerTrackerRawHit", digi_cfg));

    TrackerHitReconstructionConfig hit_reco_cfg;
    // change default parameters like hit_reco_cfg.time_resolution = 10;
    app->Add(new JChainFactoryGeneratorT<TrackerHitReconstruction_factory>({"TaggerTrackerRawHit"}, "TaggerTrackerHit", hit_reco_cfg));

//     app->Add(new JFactoryGeneratorT<RawTrackerHit_factory_ForwardRomanPotRawHits>());
//     app->Add(new JFactoryGeneratorT<TrackerHit_factory_ForwardRomanPotRecHits>());
//     app->Add(new JFactoryGeneratorT<ReconstructedParticle_factory_ForwardRomanPotParticles>());
}
}

