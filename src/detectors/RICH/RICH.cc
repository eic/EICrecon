// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

// #include "Photon_factory_PhotoElectrons"
#include "CherenkovParticleID_factory_IrtParticleID.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    /* transform RICH hits -> PhotoElectrons
     * - handles quantum efficiency
     * - safety factor
     * - pixel gap cuts
     */
    // app->Add(new JFactoryGeneratorT<PhotoElectron_factory_RICH>());

    /* transform PhotoElectrons to Cherenonkov Particle Identification
     */
    app->Add(new JFactoryGeneratorT<CherenkovParticleID_factory_IrtParticleID>());
  }
}
