// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/digi/PhotoMultiplierHitDigi_factory.h>
#include <global/pid/ParticleID_factory.h>

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JChainFactoryGeneratorT<PhotoMultiplierHitDigi_factory>({"DRICHHits"}, "DRICHRawHits"));

    /* TODO: transform raw RICH hits -> PhotoElectrons
     * - safety factor
     * - pixel gap cuts
     */

    /* TODO: transform PhotoElectrons to Cherenkov Particle Identification
     * - Run the Indirect Ray Tracing (IRT) algorithm
     * - Cherenkov angle measurement
     * - PID hypotheses
     */
    // app->Add(new JFactoryGeneratorT<ParticleID_factory>());
  }
}
