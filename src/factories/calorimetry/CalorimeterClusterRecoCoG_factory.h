// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Nathan Brei, Sebouh Paul, Dmitry Kalinkin, Derek Anderson

#pragma once

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterClusterRecoCoG_factory
    : public JOmniFactory<CalorimeterClusterRecoCoG_factory, CalorimeterClusterRecoCoGConfig> {

public:
  using AlgoT = eicrecon::CalorimeterClusterRecoCoG;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ProtoCluster> m_proto_input{this};
  PodioInput<edm4eic::MCRecoCalorimeterHitAssociation, true> m_mchitassocs_input{this};

  PodioOutput<edm4eic::Cluster> m_cluster_output{this};
  PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output{this};

  ParameterRef<std::string> m_energyWeight{this, "energyWeight", config().energyWeight};
  ParameterRef<double> m_samplingFraction{this, "samplingFraction", config().sampFrac};
  ParameterRef<double> m_logWeightBase{this, "logWeightBase", config().logWeightBase};
  ParameterRef<std::vector<double>> m_logWeightBaseCoeffs{this, "logWeightBaseCoeffs",
                                                          config().logWeightBaseCoeffs};
  ParameterRef<double> m_logWeightBase_Eref{this, "logWeightBase_Eref",
                                            config().logWeightBase_Eref};
  ParameterRef<bool> m_enableEtaBounds{this, "enableEtaBounds", config().enableEtaBounds};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_proto_input(), m_mchitassocs_input()},
                    {m_cluster_output().get(), m_assoc_output().get()});
  }
};

} // namespace eicrecon
