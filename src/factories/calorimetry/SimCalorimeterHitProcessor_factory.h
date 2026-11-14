// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/SimCalorimeterHitProcessor.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class SimCalorimeterHitProcessor_factory
    : public JOmniAlgorithmFactory<SimCalorimeterHitProcessor_factory,
                                   eicrecon::SimCalorimeterHitProcessor,
                                   SimCalorimeterHitProcessorConfig> {
private:
  ParameterRef<std::vector<double>> m_attenuationParameters{this, "attenuationParameters",
                                                            config().attenuationParameters};
  ParameterRef<std::string> m_attenuationReferencePositionName{
      this, "attenuationReferencePositionName", config().attenuationReferencePositionName};
  ParameterRef<std::vector<std::string>> m_hitMergeFields{this, "hitMergeFields",
                                                          config().hitMergeFields};
  ParameterRef<std::vector<std::string>> m_contributionMergeFields{
      this, "contributionMergeFields", config().contributionMergeFields};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<double> m_inversePropagationSpeed{this, "inversePropagationSpeed",
                                                 config().inversePropagationSpeed};
  ParameterRef<double> m_fixedTimeDelay{this, "fixedTimeDelay", config().fixedTimeDelay};
  ParameterRef<double> m_timeWindow{this, "timeWindow", config().timeWindow};
};

} // namespace eicrecon
