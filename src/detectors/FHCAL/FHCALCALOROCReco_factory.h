// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/IDDescriptor.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/calorimetry/CalorimeterCALOROCSingleEndedReco.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

struct FHCALCALOROCRecoConfig {
  CalorimeterCALOROCSingleEndedRecoConfig reco{};
  double adcToEnergy5Sum{reco.adcToEnergy};
  double adcToEnergy10Sum{reco.adcToEnergy};
  std::vector<double> timeWalkCorrectionParameters5Sum{0, 0, 0, 1};
  std::vector<double> timeWalkCorrectionParameters10Sum{0, 0, 0, 1};
};

// Reconstruct LFHCAL channels with separately configured 5-sum and 10-sum algorithms.
class FHCALCALOROCReco_factory
    : public JOmniFactory<FHCALCALOROCReco_factory, FHCALCALOROCRecoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterCALOROCSingleEndedReco;

private:
  std::unique_ptr<AlgoT> m_5sum_algorithm;
  std::unique_ptr<AlgoT> m_10sum_algorithm;

  PodioInput<edm4eic::RawCALOROCHit> m_caloroc_hits_input{this};
  PodioInput<edm4eic::SimPulse> m_pulses_input{this};
  PodioInput<edm4hep::SimCalorimeterHit> m_source_hits_input{this};
  PodioOutput<edm4eic::CalorimeterHit> m_reco_hits_output{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_links_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_assocs_output{this};

  ParameterRef<std::string> m_calorocType{this, "calorocType", config().reco.calorocType};
  ParameterRef<double> m_adcToEnergy5Sum{this, "adcToEnergy5Sum", config().adcToEnergy5Sum};
  ParameterRef<double> m_adcToEnergy10Sum{this, "adcToEnergy10Sum", config().adcToEnergy10Sum};
  ParameterRef<std::vector<double>> m_totEnergyParameters{this, "totEnergyParameters",
                                                          config().reco.totEnergyParameters};
  ParameterRef<double> m_pedestalADC{this, "pedestalADC", config().reco.pedestalADC};
  ParameterRef<bool> m_timeWalkCor{this, "timeWalkCor", config().reco.timeWalkCor};
  ParameterRef<std::vector<double>> m_timeWalkCorrectionParameters5Sum{
      this, "timeWalkCorrectionParameters5Sum", config().timeWalkCorrectionParameters5Sum};
  ParameterRef<std::vector<double>> m_timeWalkCorrectionParameters10Sum{
      this, "timeWalkCorrectionParameters10Sum", config().timeWalkCorrectionParameters10Sum};
  ParameterRef<std::string> m_samplingFraction{this, "samplingFraction", config().reco.sampFrac};
  ParameterRef<std::string> m_readout{this, "readout", config().reco.readout};
  ParameterRef<std::string> m_layerField{this, "layerField", config().reco.layerField};

  ParameterRef<double> m_time_window{this, "timeWindow", config().reco.caloroc.time_window};
  ParameterRef<unsigned int> m_calorocCapADC{this, "calorocCapADC", config().reco.caloroc.capADC};
  ParameterRef<double> m_dyRangeSingleGainADC{this, "dyRangeSingleGainADC",
                                              config().reco.caloroc.dyRangeSingleGainADC};
  ParameterRef<double> m_dyRangeHighGainADC{this, "dyRangeHighGainADC",
                                            config().reco.caloroc.dyRangeHighGainADC};
  ParameterRef<double> m_dyRangeLowGainADC{this, "dyRangeLowGainADC",
                                           config().reco.caloroc.dyRangeLowGainADC};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().reco.caloroc.capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().reco.caloroc.dyRangeTOA};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().reco.caloroc.capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().reco.caloroc.dyRangeTOT};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};
  Service<DD4hep_service> m_geoSvc{this};
  dd4hep::BitFieldCoder* m_decoder = nullptr;
  std::size_t m_layer_idx          = 0;

public:
  void Configure() {
    const auto& reco = config().reco;
    m_decoder        = m_geoSvc().detector()->readout(reco.readout).idSpec().decoder();
    m_layer_idx      = m_decoder->index(reco.layerField);

    auto reco_config                         = reco;
    reco_config.adcToEnergy                  = config().adcToEnergy5Sum;
    reco_config.timeWalkCorrectionParameters = config().timeWalkCorrectionParameters5Sum;
    m_5sum_algorithm                         = std::make_unique<AlgoT>(GetPrefix() + "_5sum");
    m_5sum_algorithm->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_5sum_algorithm->applyConfig(reco_config);
    m_5sum_algorithm->init();

    reco_config.adcToEnergy                  = config().adcToEnergy10Sum;
    reco_config.timeWalkCorrectionParameters = config().timeWalkCorrectionParameters10Sum;
    m_10sum_algorithm                        = std::make_unique<AlgoT>(GetPrefix() + "_10sum");
    m_10sum_algorithm->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_10sum_algorithm->applyConfig(reco_config);
    m_10sum_algorithm->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // Route aligned hits and pulses in input order, preserving output and truth-link ordering.
    for (std::size_t i = 0; i < m_caloroc_hits_input()->size(); ++i) {
      const auto hit  = (*m_caloroc_hits_input())[i];
      auto* algorithm = m_5sum_algorithm.get();
      if (m_decoder->get(hit.getCellID(), m_layer_idx) >= 4) {
        algorithm = m_10sum_algorithm.get();
      }

      edm4eic::RawCALOROCHitCollection input_hits;
      input_hits.setSubsetCollection(true);
      input_hits.push_back(hit);
      edm4eic::SimPulseCollection input_pulses;
      input_pulses.setSubsetCollection(true);
      input_pulses.push_back((*m_pulses_input())[i]);
      algorithm->process({&input_hits, &input_pulses, m_source_hits_input()},
                         {m_reco_hits_output().get(), m_hits_output().get(), m_links_output().get(),
                          m_hit_assocs_output().get()});
    }
  }
};

} // namespace eicrecon
