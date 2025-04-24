// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/TensorCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/fardetectors/FarDetectorTransportationPreMLConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using FarDetectorTransportationPreMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackParametersCollection,
                      std::optional<edm4hep::MCParticleCollection>,
                      std::optional<edm4hep::MCParticleCollection>>,
    algorithms::Output<edm4eic::TensorCollection, std::optional<edm4eic::TensorCollection>>>;

class FarDetectorTransportationPreML : public FarDetectorTransportationPreMLAlgorithm,
                                       public WithPodConfig<FarDetectorTransportationPreMLConfig> {

public:
  FarDetectorTransportationPreML(std::string_view name)
      : FarDetectorTransportationPreMLAlgorithm{
            name,
            {"TrackParameters", "ScatteredElectrons", "BeamElectrons"},
            {"outputFeatureTensor", "outputTargetTensor"},
            "Create tensor for input to far-detector magnetic transportation ML."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  mutable float m_beamE = 10.0;
  mutable std::once_flag m_initBeamE;
};

} // namespace eicrecon
