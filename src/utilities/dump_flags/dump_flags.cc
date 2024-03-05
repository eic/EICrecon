// SPDX-License-Identifier: JSA
// Copyright (C) 2022, Dmitry Romanov

#include <JANA/JApplication.h>

#include "DumpFlags_processor.h"


extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new DumpFlags_processor(app));
    }
}
