// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "Log_service.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->ProvideService(std::make_shared<Log_service>(app) );
}
}
