// Copyright 2023, Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include <extensions/jana/JChainFactoryGeneratorT.h>

#include "OffMomentumReconstruction_factory.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;


    app->Add(new JFactoryGeneratorT<OffMomentumReconstruction_factory>());
}
}
