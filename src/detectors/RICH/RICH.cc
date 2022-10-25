// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

// #include "Photon_factory_PhotoElectrons"
#include "ParticleID_factory_IrtHypothesis.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    /* transform RICH hits -> PhotoElectrons
     * - handles quantum efficiency
     * - safety factor
     * - pixel gap cuts
     */
    // app->Add(new JFactoryGeneratorT<PhotoElectron_factory_RICH>());

    /* transform PhotoElectrons to Cherenkov Particle Identification
     * - Run the Indirect Ray Tracing (IRT) algorithm
     * - Cherenkov angle measurement
     * - PID hypotheses
     */
    app->Add(new JFactoryGeneratorT<ParticleID_factory_IrtHypothesis>());
  }
}
