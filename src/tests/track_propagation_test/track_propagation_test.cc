// SPDX-License-Identifier: JSA
// Copyright (C) 2022, David Lawrence

#include <JANA/JApplication.h>

#include "TrackPropagationTest_processor.h"


extern "C" {
    void InitPlugin(JApplication *app) {

        // Initializes this plugin
        InitJANAPlugin(app);

        // Adds our processor to JANA2 to execute
        app->Add(new TrackPropagationTest_processor(app));
    }
}
