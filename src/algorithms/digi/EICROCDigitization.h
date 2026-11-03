// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from EICROCGeneration into ADC and TDC values

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

#include "algorithms/digi/EICROCDigitizationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using EICROCDigitizationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::RawTimeSeriesCollection, edm4eic::SimPulseCollection>,
    algorithms::Output<edm4eic::RawTrackerHitCollection, edm4eic::MCRecoTrackerHitLinkCollection,
                       edm4eic::MCRecoTrackerHitAssociationCollection>>;

class EICROCDigitization : public EICROCDigitizationAlgorithm,
                           public WithPodConfig<EICROCDigitizationConfig> {

public:
  EICROCDigitization(std::string_view name)
      : EICROCDigitizationAlgorithm{
            name,
            {"EICROC", "EICROCTruth"},
            {"ADCTDCOutput", "ADCTDCOutputLinks", "ADCTDCOutputAssociations"},
            "Digitize waveforms into ADC and TDC values. Raw hits are linked to the sim hits "
            "of the SimPulse at the waveform's index."} {}
  void init() {};
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
