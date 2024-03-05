// SPDX-License-Identifier: JSA
// Copyright (C) 2022, David Lawrence

#include <JANA/JApplication.h>

#include "GlobalReconstructionTest_processor.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new GlobalReconstructionTest_processor(app));
    }
}
