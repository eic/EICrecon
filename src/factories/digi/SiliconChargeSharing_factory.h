// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Chun Yuen Tsang, Simon Gardner

#pragma once

#include "algorithms/digi/SiliconChargeSharing.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class SiliconChargeSharing_factory
    : public JOmniAlgorithmFactory<SiliconChargeSharing_factory, eicrecon::SiliconChargeSharing,
                                   SiliconChargeSharingConfig> {
private:
  ParameterRef<float> m_sigma_sharingx{this, "sigmaSharingX", config().sigma_sharingx};
  ParameterRef<float> m_sigma_sharingy{this, "sigmaSharingY", config().sigma_sharingy};
  ParameterRef<float> m_min_edep{this, "minEDep", config().min_edep};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<eicrecon::SiliconChargeSharingConfig::ESigmaMode> m_sigma_mode{this, "sigmaMode",
                                                                              config().sigma_mode};
};
} // namespace eicrecon
