// SPDX-License-Identifier: JSA
// Copyright (C) 2022 David Lawrence
//

#include <JANA/JApplication.h>
#include <memory>

#include "RootFile_service.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->ProvideService(std::make_shared<RootFile_service>(app) );
}
}
