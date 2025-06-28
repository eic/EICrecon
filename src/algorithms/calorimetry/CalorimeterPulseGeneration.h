// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>

#include "CalorimeterPulseGenerationConfig.h"
#include "algorithms/digi/SiliconPulseGeneration.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType = edm4eic::SimPulse;
#else
using PulseType = edm4hep::TimeSeries;
#endif

using CalorimeterPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimCalorimeterHitCollection>,
                          algorithms::Output<PulseType::collection_type>>;

class CalorimeterPulseGeneration : public CalorimeterPulseGenerationAlgorithm,
                                   public WithPodConfig<CalorimeterPulseGenerationConfig> {

public:
  CalorimeterPulseGeneration(std::string_view name)
      : CalorimeterPulseGenerationAlgorithm{
            name,
            {"inputHitCollection"},
            {"outputPulseCollection"},
            "Add hits for each readout and generate corresponding pulse"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<SignalPulse> m_pulse;
  double m_min_sampling_time;
  double m_ignore_thres;

  std::optional<double> m_edep_to_npe;

  mutable std::mt19937 m_gen{};
};

} // namespace eicrecon
