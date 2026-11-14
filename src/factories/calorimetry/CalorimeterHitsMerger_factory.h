// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterHitsMerger_factory
    : public JOmniAlgorithmFactory<CalorimeterHitsMerger_factory, eicrecon::CalorimeterHitsMerger,
                                   CalorimeterHitsMergerConfig> {
private:
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::vector<std::string>> m_field_transformations{this, "fieldTransformations",
                                                                 config().fieldTransformations};
};

} // namespace eicrecon
