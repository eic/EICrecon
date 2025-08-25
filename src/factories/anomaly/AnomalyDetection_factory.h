// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 EIC Reconstruction Contributors

#pragma once

#include <algorithms/anomaly/AnomalyDetection.h>
#include <extensions/jana/JOmniFactoryGeneratorT.h>

namespace eicrecon {

class AnomalyDetection_factory : public JOmniFactoryGeneratorT<AnomalyDetection> {

public:
  using AlgoT    = AnomalyDetection;
  using FactoryT = JOmniFactoryGeneratorT<AlgoT>;

  AnomalyDetection_factory() {
    SetPluginName("AnomalyDetection");
    SetTypeName(NAME_OF_THIS);
    SetTag("AnomalyDetection");
    SetLevel(JEventLevel::PhysicsEvent);

    // Input collections
    SetInputTags("inputMCParticles", {"MCParticles"});
    SetInputTags("inputReconstructedParticles", {"ReconstructedParticles"});

    // No output collections - results are sent to audio service

    // Algorithm configuration
    SetCallbackStyle(JOmniFactoryGeneratorT<AlgoT>::CallbackStyle::Create);

    // Default configuration
    GetConfig()["detector_systems"]   = {"BEMC", "BHCAL", "EEMC", "EHCAL", "FEMC",   "FHCAL",
                                         "BTRK", "ECTRK", "BVTX", "DRICH", "PFRICH", "DIRC",
                                         "BTOF", "ECTOF", "ZDC",  "B0TRK", "B0ECAL"};
    GetConfig()["energy_threshold"]   = 0.1;
    GetConfig()["momentum_threshold"] = 0.1;
    GetConfig()["max_anomaly_value"]  = 10.0;
    GetConfig()["update_frequency"]   = 10;
  }

  void Configure() {
    auto& cfg = GetConfig();

    m_algo_config.detector_systems   = cfg["detector_systems"].get<std::vector<std::string>>();
    m_algo_config.energy_threshold   = cfg["energy_threshold"].get<double>();
    m_algo_config.momentum_threshold = cfg["momentum_threshold"].get<double>();
    m_algo_config.max_anomaly_value  = cfg["max_anomaly_value"].get<double>();
    m_algo_config.update_frequency   = cfg["update_frequency"].get<int>();

    SetAlgorithmConfig(m_algo_config);
  }

  void ChangeRun(int64_t run_number) {}
  void Process(int64_t run_number, uint64_t event_number) {}

private:
  AnomalyDetectionConfig m_algo_config;
};

} // namespace eicrecon
