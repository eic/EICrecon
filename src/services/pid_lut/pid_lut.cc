// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JApplication.h>
#include "PIDLookupTable_service.h"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->ProvideService(std::make_shared<PIDLookupTable_service>());
}
}
