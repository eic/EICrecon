// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/digi/SiliconPulseDiscretization.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class SiliconPulseDiscretization_factory
    : public JOmniAlgorithmFactory<SiliconPulseDiscretization_factory,
                                   eicrecon::SiliconPulseDiscretization,
                                   SiliconPulseDiscretizationConfig> {
private:
  ParameterRef<double> m_EICROC_period{this, "EICROCPeriod", config().EICROC_period};
  ParameterRef<double> m_local_period{this, "localPeriod", config().local_period};
  ParameterRef<double> m_global_offset{this, "globalOffset", config().global_offset};
};

} // namespace eicrecon
