// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "JFactory_EcalBarrelRawCalorimeterHit.h"
#include "JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits.h"
    
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<JFactory_EcalBarrelRawCalorimeterHit>());
        app->Add(new JFactoryGeneratorT<JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits>());
    }
}
    
