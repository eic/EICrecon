// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawTrackerHit_factory_ForwardRomanPotRawHits.h"
#include "TrackerHit_factory_ForwardRomanPotRecHits.h"
#include "ReconstructedParticle_factory_ForwardRomanPotParticles.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JFactoryGeneratorT<RawTrackerHit_factory_ForwardRomanPotRawHits>());
    app->Add(new JFactoryGeneratorT<TrackerHit_factory_ForwardRomanPotRecHits>());
    app->Add(new JFactoryGeneratorT<ReconstructedParticle_factory_ForwardRomanPotParticles>());
}
}

