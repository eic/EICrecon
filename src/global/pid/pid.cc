// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <JANA/JFactoryGenerator.h>

#include "ParticleID_factory.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JFactoryGeneratorT<ParticleID_factory>());
  }
}
