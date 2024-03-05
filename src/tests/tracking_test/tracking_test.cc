// SPDX-License-Identifier: JSA
// Copyright (C) 2022, David Lawrence

#include <JANA/JApplication.h>

#include "TrackingTest_processor.h"
//#include "JFactory_EcalBarrelRawCalorimeterHit.h.bck"
//#include "JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits.h.bck"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new TrackingTest_processor(app));
        //app->Add(new JFactoryGeneratorT<JFactory_EcalBarrelRawCalorimeterHit>());
        //app->Add(new JFactoryGeneratorT<JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits>());
    }
}
