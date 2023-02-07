// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//


#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <JANA/JFactoryGenerator.h>

#include "ReconstructedParticle_factory_SmearedFarForwardParticles.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JFactoryGeneratorT<ReconstructedParticle_factory_SmearedFarForwardParticles>());

}
} // extern "C"
