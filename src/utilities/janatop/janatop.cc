// SPDX-License-Identifier: JSA
// Copyright (C) 2022, Dmitry Romanov

#include <JANA/JApplication.h>
#include <JANA/Services/JParameterManager.h>
#include <memory>

#include "JEventProcessorJANATOP.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JEventProcessorJANATOP());
        app->GetJParameterManager()->SetParameter("RECORD_CALL_STACK", true);
    }
}
