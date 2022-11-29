// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>

#include "IrtParticleID_factory.h"
#include "RichTrack_factory.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>({"CentralCKFTrajectories"}, "DRICHTracks"));
    app->Add(new JFactoryGeneratorT<IrtParticleID_factory>());
  }
}
