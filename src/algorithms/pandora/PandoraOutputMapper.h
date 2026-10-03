// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <Api/PandoraApi.h>
#include <Objects/ParticleFlowObject.h>
#include <Pandora/Pandora.h>
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/ReconstructedParticleCollection.h>

namespace eicrecon {

/// Translates Pandora ParticleFlowObjects into edm4eic::ReconstructedParticles.
class PandoraOutputMapper {
public:
  /// Convert all PFOs from the current Pandora event into @p outputParticles.
  ///
  /// @param pandora          The Pandora instance after ProcessEvent() has been called.
  /// @param outputParticles  Collection to receive the output reconstructed particles.
  static void retrievePFOs(const pandora::Pandora& pandora,
                           edm4eic::ReconstructedParticleCollection& outputParticles);
}; // end PandoraOutputMapper

} // namespace eicrecon
