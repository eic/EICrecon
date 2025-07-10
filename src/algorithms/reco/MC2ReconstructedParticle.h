// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using MC2ReconstructedParticleAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

/**
  * Converts edm4hep::MCParticle to edm4eic::ReconstructedParticle
  */
class MC2ReconstructedParticle : public MC2ReconstructedParticleAlgorithm,
                                 public WithPodConfig<NoConfig> {
public:
  MC2ReconstructedParticle(std::string_view name)
      : MC2ReconstructedParticleAlgorithm{name,
                                          {"inputMCParticles"},
                                          {"outputReconstructedParticles"},
                                          "Convert truth MC particles into reconstructed particles,"
                                          "optionally applying momentum smearing."} {}

  /** Initialized algorithms with required data. Init function is assumed to be called once **/
  void init() final;

  /** process function convert one data type to another **/
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
