// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

    }
}
    
