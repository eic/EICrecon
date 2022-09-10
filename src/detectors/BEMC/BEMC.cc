// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalBarrelNRawHits.h"
//#include "BEMCRawCalorimeterHit_factory_utility.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelNRawHits>());
        //app->Add(new JFactoryGeneratorT<BEMCRawCalorimeterHit_factory_utility>());
    }
}
    
