// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <edm4eic/ReconstructedParticle.h>
#include <fmt/core.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/CollectionCollector_factory.h"

#include "algorithms/particle/ParticleConverter.h"
#include "factories/particle/ParticleConverter_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  // ASK ABOUT THIS SECOND ARGUMENT -> FALSE
  app->Add(new JOmniFactoryGeneratorT<ParticleConverter_factory>(
      "FinalReconstructedParticles", {"ReconstructedParticles"}, {"FinalReconstructedParticles"},
      {}, app));
}
} // extern "C"
