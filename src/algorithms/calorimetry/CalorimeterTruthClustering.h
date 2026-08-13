
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterTruthClusteringAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection,
                                            edm4eic::MCRecoCalorimeterHitLinkCollection>,
                          algorithms::Output<edm4eic::ProtoClusterCollection>>;

class CalorimeterTruthClustering : public CalorimeterTruthClusteringAlgorithm,
                                   public WithPodConfig<NoConfig> {

public:
  CalorimeterTruthClustering(std::string_view name)
      : CalorimeterTruthClusteringAlgorithm{name,
                                            {"inputHitCollection", "inputHitLinks"},
                                            {"outputProtoClusterCollection"},
                                            "Use truth information for clustering."} {}

public:
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  edm4hep::MCParticle get_primary(const edm4hep::CaloHitContribution& contrib) const;
};

} // namespace eicrecon
