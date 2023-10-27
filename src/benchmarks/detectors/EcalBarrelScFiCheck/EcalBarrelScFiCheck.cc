#include <JANA/JApplication.h>

#include "EcalBarrelScFiCheckProcessor.h"

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new EcalBarrelScFiCheckProcessor);
    }
}
