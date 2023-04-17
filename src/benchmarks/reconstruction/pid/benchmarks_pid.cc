// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "Digitizer_processor.h"
#include "IrtCherenkovParticleID_processor.h"
// #include "LinkParticleID_processor.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new eicrecon::Digitizer_processor);
    app->Add(new eicrecon::IrtCherenkovParticleID_processor);
    // app->Add(new eicrecon::LinkParticleID_processor);
  }
}
