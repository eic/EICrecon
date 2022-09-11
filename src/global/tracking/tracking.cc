// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>
#include "TrackerSourceLinker_factory.h"



extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
//    app->Add(new JChainFactoryGeneratorT<TrackerSourceLinker_factory>(
//            {"SagittaSiBarrelHits", "OuterSiBarrelHits"},
//            "BarrelTrackerRawHit"));

}
} // extern "C"

