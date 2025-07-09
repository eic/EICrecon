#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "services/log/Log.h"
#include "algorithms/calorimetry/CalorimeterEoverPCut.h"

namespace eicrecon {

struct CalorimeterEoverPCutConfig {
    int    max_layer = 8;  // how many layers to integrate (unused in this simple cut)
    double ecut      = 0.74; // E/P threshold
};

class CalorimeterEoverPCut_factory
  : public JOmniFactory<CalorimeterEoverPCut_factory, CalorimeterEoverPCutConfig> {
public:
  using AlgoT = EoverPCutAlgorithm;

private:
  std::unique_ptr<AlgoT> m_algo;

  // Input ports: whole Cluster collection and its MC associations
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_input{this};

  // Output port: a stream of ParticleID objects
  PodioOutput<edm4hep::ParticleID> m_pid_output{this};

public:
  void Configure() override {
    // instantiate & configure the algorithm
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /*run*/) override {}

  void Process(const algorithm::InputTag&, const algorithm::ProcessingContext&) override {
    // feed it the inputs and collect the ParticleID outputs
    m_algo->process({ m_clusters_input(), m_assoc_input() },
                    { m_pid_output() });
  }
};

} // namespace eicrecon
