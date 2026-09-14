// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <string_view>

#include "algorithms/calorimetry/CALOROCToRawCalorimeterHitConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CALOROCToRawCalorimeterHitAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::RawCALOROCHitCollection, edm4eic::SimPulseCollection>,
    algorithms::Output<edm4hep::RawCalorimeterHitCollection,
                       edm4eic::MCRecoCalorimeterHitLinkCollection,
                       edm4eic::MCRecoCalorimeterHitAssociationCollection>>;

class CALOROCToRawCalorimeterHit : public CALOROCToRawCalorimeterHitAlgorithm,
                                  public WithPodConfig<CALOROCToRawCalorimeterHitConfig> {

public:
  CALOROCToRawCalorimeterHit(std::string_view name)
      : CALOROCToRawCalorimeterHitAlgorithm{
            name,
            {"inputCALOROCHits", "inputPulses"},
            {"outputRawHits", "outputHitLinks", "outputRawHitAssociations"},
            {"Converts CALOROC samples into the legacy calorimeter raw-hit interface"}} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  double m_stepTDC{0};
};

} // namespace eicrecon
