// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <algorithms/algorithm.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <string_view>

#include "algorithms/calorimetry/EdepToSiPMConversionConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using EdepToSiPMConversionAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimCalorimeterHitCollection>,
    algorithms::Output<edm4hep::SimCalorimeterHitCollection>>;

class EdepToSiPMConversion : public EdepToSiPMConversionAlgorithm,
                            public WithPodConfig<EdepToSiPMConversionConfig> {

public:
  EdepToSiPMConversion(std::string_view name)
      : EdepToSiPMConversionAlgorithm{name, {"EventHeader", "inputHits"}, {"outputHits"}, {}} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
