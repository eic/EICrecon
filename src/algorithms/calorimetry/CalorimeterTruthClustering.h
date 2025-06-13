
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <string>
#include <string_view>

namespace eicrecon {

using CalorimeterTruthClusteringAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::CalorimeterHitCollection, edm4hep::SimCalorimeterHitCollection>,
    algorithms::Output<edm4eic::ProtoClusterCollection>>;

class CalorimeterTruthClustering : public CalorimeterTruthClusteringAlgorithm {

public:
  CalorimeterTruthClustering(std::string_view name)
      : CalorimeterTruthClusteringAlgorithm{name,
                                            {"inputHitCollection", "inputSimHitCollection"},
                                            {"outputProtoClusterCollection"},
                                            "Use truth information for clustering."} {}

public:
  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
