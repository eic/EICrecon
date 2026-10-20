// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <memory>
#include <string>

#include "algorithms/digi/CALOROCDigitization.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

struct FHCALCALOROCConfig {
  CALOROCDigitizationConfig caloroc{};

  double toa_thres_5sum{1};
  double toa_thres_10sum{1};
  unsigned int toa_layer_offset{0};
  unsigned int toa_layer_mask{0};
};

// Apply layer thresholds through the shared CALOROC digitizer, then retain matched source pulses.
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
  ParameterRef<unsigned int> m_toa_layer_offset{this, "toaLayerOffset", config().toa_layer_offset};
  ParameterRef<unsigned int> m_toa_layer_mask{this, "toaLayerMask", config().toa_layer_mask};
  ParameterRef<double> m_tot_thres{this, "totThres", config().caloroc.tot_thres};
  ParameterRef<unsigned int> m_capADC{this, "capADC", config().caloroc.capADC};
  ParameterRef<double> m_dyRangeSingleGainADC{this, "dyRangeSingleGainADC", config().caloroc.dyRangeSingleGainADC};
  ParameterRef<double> m_dyRangeHighGainADC{this, "dyRangeHighGainADC", config().caloroc.dyRangeHighGainADC};
  ParameterRef<double> m_dyRangeLowGainADC{this, "dyRangeLowGainADC", config().caloroc.dyRangeLowGainADC};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().caloroc.capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().caloroc.dyRangeTOA};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().caloroc.capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().caloroc.dyRangeTOT};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {

    // Configure the digitizer for 5-sum channels.
    auto caloroc = config().caloroc;
    caloroc.toa_thres = config().toa_thres_5sum;
    m_5sum_algorithm = std::make_unique<AlgoT>(GetPrefix() + "_5sum");
    m_5sum_algorithm->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_5sum_algorithm->applyConfig(caloroc);
    m_5sum_algorithm->init();

    // Configure the digitizer for 10-sum channels with the same sampling settings.
    caloroc.toa_thres = config().toa_thres_10sum;
    m_10sum_algorithm = std::make_unique<AlgoT>(GetPrefix() + "_10sum");
    m_10sum_algorithm->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_10sum_algorithm->applyConfig(caloroc);
    m_10sum_algorithm->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    // Retain references to the noisy input pulses used by reconstruction.
    m_out_pulses()->setSubsetCollection(true);

    // Loop over noisy pulses in input order.
    for (const auto pulse : *m_in_pulses()) {

      // Layers 0-3 use 5-sum channels; layers 4-7 use 10-sum channels.
      const auto layer = (pulse.getCellID() >> config().toa_layer_offset) & config().toa_layer_mask;
      auto* algorithm = m_5sum_algorithm.get();
      if (layer >= 4) {
        algorithm = m_10sum_algorithm.get();
      }

      // Digitize this pulse and append its hit to the event output.
      edm4eic::SimPulseCollection input_pulses;
      input_pulses.setSubsetCollection(true);
      input_pulses.push_back(pulse);
      const auto previous_size = m_out_hits()->size();
      algorithm->process({&input_pulses}, {m_out_hits().get()});

      // Save the source pulse when a hit is produced so reco inputs stay aligned.
      if (m_out_hits()->size() > previous_size) {
        m_out_pulses()->push_back(pulse);
      }
    }
  }
};

} // namespace eicrecon
