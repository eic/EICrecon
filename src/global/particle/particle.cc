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

  app->Add(new JOmniFactoryGeneratorT<ParticleConverter_factory>(
      "FinalReconstructedParticles", 
      {"ReconstructedParticles"}, 
      {"FinalReconstructedParticles"},
      {
        .ecal_resolution   = 1.0,
        .hcal_resolution   = 1.0,
        .calo_hadron_scale = 1.0,
        .calo_energy_norm  = 1.0,
      }, 
      app));
}
} // extern "C"
