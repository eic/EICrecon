// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include "algorithms/digi/CALOROCDigitization.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CALOROCDigitization_factory
    : public JOmniAlgorithmFactory<CALOROCDigitization_factory, eicrecon::CALOROCDigitization,
                                   CALOROCDigitizationConfig> {
private:
  ParameterRef<double> m_time_window{this, "timeWindow", config().time_window};
  ParameterRef<double> m_adc_phase{this, "adcPhase", config().adc_phase};
  ParameterRef<double> m_toa_thres{this, "toaThres", config().toa_thres};
  ParameterRef<double> m_tot_thres{this, "totThres", config().tot_thres};
  ParameterRef<unsigned int> m_capADC{this, "capADC", config().capADC};
  ParameterRef<double> m_dyRangeSingleGainADC{this, "dyRangeSingleGainADC",
                                              config().dyRangeSingleGainADC};
  ParameterRef<double> m_dyRangeHighGainADC{this, "dyRangeHighGainADC",
                                            config().dyRangeHighGainADC};
  ParameterRef<double> m_dyRangeLowGainADC{this, "dyRangeLowGainADC", config().dyRangeLowGainADC};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().dyRangeTOA};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().dyRangeTOT};
};
} // namespace eicrecon
