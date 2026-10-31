// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/IDDescriptor.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <memory>
#include <string>

#include "algorithms/digi/CALOROCDigitization.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

struct FHCALCALOROCConfig {
  CALOROCDigitizationConfig caloroc{};

  double toa_thres_5sum{1};
  double toa_thres_10sum{1};
  std::string readout;
  std::string layer_field;
};

// Digitize pulses with layer-dependent thresholds.
class FHCALCALOROC_factory : public JOmniFactory<FHCALCALOROC_factory, FHCALCALOROCConfig> {
public:
  using AlgoT = eicrecon::CALOROCDigitization;

private:
  std::unique_ptr<AlgoT> m_5sum_algorithm;
  std::unique_ptr<AlgoT> m_10sum_algorithm;

  PodioInput<edm4eic::SimPulse> m_in_pulses{this};
  PodioOutput<edm4eic::RawCALOROCHit> m_out_hits{this};
  PodioOutput<edm4eic::SimPulse> m_out_pulses{this};

  ParameterRef<double> m_time_window{this, "timeWindow", config().caloroc.time_window};
  ParameterRef<double> m_adc_phase{this, "adcPhase", config().caloroc.adc_phase};
  ParameterRef<double> m_toa_thres_5sum{this, "toaThres5sum", config().toa_thres_5sum};
  ParameterRef<double> m_toa_thres_10sum{this, "toaThres10sum", config().toa_thres_10sum};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_layer_field{this, "layerField", config().layer_field};
  ParameterRef<double> m_tot_thres{this, "totThres", config().caloroc.tot_thres};
  ParameterRef<double> m_pedestal{this, "pedestal", config().caloroc.pedestal};
  ParameterRef<unsigned int> m_capADC{this, "capADC", config().caloroc.capADC};
  ParameterRef<double> m_dyRangeSingleGainADC{this, "dyRangeSingleGainADC",
                                              config().caloroc.dyRangeSingleGainADC};
  ParameterRef<double> m_dyRangeHighGainADC{this, "dyRangeHighGainADC",
                                            config().caloroc.dyRangeHighGainADC};
  ParameterRef<double> m_dyRangeLowGainADC{this, "dyRangeLowGainADC",
                                           config().caloroc.dyRangeLowGainADC};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().caloroc.capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().caloroc.dyRangeTOA};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().caloroc.capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().caloroc.dyRangeTOT};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};
  Service<DD4hep_service> m_geoSvc{this};
  dd4hep::BitFieldCoder* m_decoder = nullptr;
  std::size_t m_layer_idx          = 0;

public:
  void Configure() {
    if (!config().layer_field.empty()) {
      m_decoder   = m_geoSvc().detector()->readout(config().readout).idSpec().decoder();
      m_layer_idx = m_decoder->index(config().layer_field);
    }

    // Configure the digitizer for 5-sum channels.
    auto caloroc      = config().caloroc;
    caloroc.toa_thres = config().toa_thres_5sum;
    m_5sum_algorithm  = std::make_unique<AlgoT>(GetPrefix() + "_5sum");
    m_5sum_algorithm->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_5sum_algorithm->applyConfig(caloroc);
    m_5sum_algorithm->init();

    // Configure the digitizer for 10-sum channels.
    caloroc.toa_thres = config().toa_thres_10sum;
    m_10sum_algorithm = std::make_unique<AlgoT>(GetPrefix() + "_10sum");
    m_10sum_algorithm->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_10sum_algorithm->applyConfig(caloroc);
    m_10sum_algorithm->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    m_out_pulses()->setSubsetCollection(true);

    // Loop over pulses.
    for (const auto pulse : *m_in_pulses()) {

      // Layers 0-3 use 5-sum channels; layers 4-7 use 10-sum channels.
      const auto layer = m_decoder == nullptr ? 0 : m_decoder->get(pulse.getCellID(), m_layer_idx);
      auto* algorithm  = m_5sum_algorithm.get();
      if (layer >= 4) {
        algorithm = m_10sum_algorithm.get();
      }

      edm4eic::SimPulseCollection input_pulses;
      input_pulses.setSubsetCollection(true);
      input_pulses.push_back(pulse);
      algorithm->process({&input_pulses}, {m_out_hits().get(), m_out_pulses().get()});
    }
  }
};

} // namespace eicrecon
