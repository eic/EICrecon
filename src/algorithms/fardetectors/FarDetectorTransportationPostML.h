// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Simon Gardner

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/fardetectors/FarDetectorTransportationPostMLConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using FarDetectorTransportationPostMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TensorCollection, std::optional<edm4hep::MCParticleCollection>>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class FarDetectorTransportationPostML
    : public FarDetectorTransportationPostMLAlgorithm,
      public WithPodConfig<FarDetectorTransportationPostMLConfig> {

public:
  FarDetectorTransportationPostML(std::string_view name)
      : FarDetectorTransportationPostMLAlgorithm{
            name,
            {"inputPredictionsTensor"},
            {"outputParticles"},
            "Convert ML output tensor into reconstructed electron"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  double m_mass         = 0.000511; // Default to electron mass in GeV
  float m_charge        = -1.0;     // Default to electron charge
  mutable float m_beamE = 10.0;
  mutable std::once_flag m_initBeamE;
};

} // namespace eicrecon
