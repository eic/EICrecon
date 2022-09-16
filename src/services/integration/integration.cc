// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "Algorithms_service.h"

// Make this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->ProvideService(std::make_shared<eicrecon::Algorithms_service>(app) );
    }
}

