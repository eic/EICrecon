// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#include <JANA/JApplication.h>
#include "PIDLookupTable_service.h"
#include <memory>


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->ProvideService(std::make_shared<PIDLookupTable_service>());
}
}
