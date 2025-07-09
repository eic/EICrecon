#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include "factories/calorimetry/CalorimeterEoverPCut_factory.h"

namespace eicrecon {

// Define base type: inputs = Clusters + optional Assocs, output = ParticleIDCollection
using CalorimeterEoverPCutAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>,
    algorithms::Output<edm4hep::ParticleIDCollection>>;

// E/P Cut algorithm implementing the base and reading config from factory
class CalorimeterEoverPCut : public CalorimeterEoverPCutAlgorithm,
                             public WithPodConfig<CalorimeterEoverPCutConfig> {
public:
  // name: factory prefix, inputs: "inputClusters","inputAssocs", outputs: "outputPIDs"
  CalorimeterEoverPCut(std::string_view name)
    : CalorimeterEoverPCutAlgorithm{name,
        {"inputClusters","inputAssocs"},
        {"outputPIDs"},
        "E/P Cut Algorithm"},
      WithPodConfig<CalorimeterEoverPCutConfig>{} {}

  void init() final {}
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
