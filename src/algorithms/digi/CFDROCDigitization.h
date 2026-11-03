// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang
//
// Convert ADC pulses from CFDROCGeneration into ADC and TDC values

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <cmath>
#include <string>
#include <string_view>

#include "algorithms/digi/CFDROCDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CFDROCDigitizationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::RawTimeSeriesCollection, edm4eic::SimPulseCollection>,
    algorithms::Output<edm4eic::RawTrackerHitCollection, edm4eic::MCRecoTrackerHitLinkCollection,
                       edm4eic::MCRecoTrackerHitAssociationCollection>>;

class CFDROCDigitization : public CFDROCDigitizationAlgorithm,
                           public WithPodConfig<CFDROCDigitizationConfig> {

public:
  CFDROCDigitization(std::string_view name)
      : CFDROCDigitizationAlgorithm{
            name,
            {"inputWaveforms", "inputWaveformTruth"},
            {"outputRawHits", "outputRawHitLinks", "outputRawHitAssociations"},
            "Discriminate and digitize signals with timing based on what is expected from a CFD. "
            "Raw hits are linked to the sim hits of the SimPulse at the waveform's index."} {}
  void init() {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
