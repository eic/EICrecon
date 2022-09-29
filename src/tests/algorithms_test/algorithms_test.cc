// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include "AlgorithmsTest_processor.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new AlgorithmsTest_processor(app));
    //app->Add(new JFactoryGeneratorT<JFactory_EcalBarrelRawCalorimeterHit>());
    //app->Add(new JFactoryGeneratorT<JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits>());
}
}
    
