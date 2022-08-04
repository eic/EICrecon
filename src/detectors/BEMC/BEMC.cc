// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "JFactory_BEMCRawCalorimeterHit.h"
#include "JFactory_BEMCRawCalorimeterHit_utility.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<JFactory_BEMCRawCalorimeterHit>());
        app->Add(new JFactoryGeneratorT<JFactory_BEMCRawCalorimeterHit_utility>());
    }
}
    
