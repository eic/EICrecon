
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterTruthClusteringAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection,
                                            edm4eic::MCRecoCalorimeterHitAssociationCollection>,
                          algorithms::Output<edm4eic::ProtoClusterCollection>>;

class CalorimeterTruthClustering : public CalorimeterTruthClusteringAlgorithm,
                                   public WithPodConfig<NoConfig> {

public:
  CalorimeterTruthClustering(std::string_view name)
      : CalorimeterTruthClusteringAlgorithm{name,
                                            {"inputHitCollection", "inputHitAssociations"},
                                            {"outputProtoClusterCollection"},
                                            "Use truth information for clustering."} {}

public:
  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
