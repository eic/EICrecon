#pragma once

#include "algorithms/calorimetry/CalorimeterEoverPCut.h"
#include "extensions/jana/JOmniFactory.h"
#include <JANA/JApplication.h> // for GetParameter

namespace eicrecon {

class CalorimeterEoverPCut_factory
    : public JOmniFactory<CalorimeterEoverPCut_factory, NoConfig> {
public:
  using AlgoT = CalorimeterEoverPCut;

private:
  std::unique_ptr<AlgoT> m_algo;

  // Exactly one‐argument PodioInput/Output
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_input{this};
  PodioOutput<edm4hep::ParticleID> m_pid_output{this};

public:
  void Configure() {
    // instantiate
    m_algo = std::make_unique<AlgoT>(GetPrefix());

    // pull params from JANA: BEMCEoverPCut:ecut , BEMCEoverPCut:max_layer
    double ecut_val = m_algo->getEcut(); // initial default
    GetApplication()->GetParameter(GetPrefix() + ":ecut", ecut_val);
    m_algo->setEcut(ecut_val);

    int maxL = m_algo->getMaxLayer(); // initial default
    GetApplication()->GetParameter(GetPrefix() + ":max_layer", maxL);
    m_algo->setMaxLayer(maxL);

    m_algo->init();
  }

  void ChangeRun(int32_t /*run_number*/) {}

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_clusters_input(), m_assoc_input()}, {m_pid_output()});
  }
};

} // namespace eicrecon
