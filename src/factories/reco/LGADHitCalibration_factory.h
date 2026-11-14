// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "algorithms/reco/LGADHitCalibration.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADHitCalibration_factory
    : public JOmniAlgorithmFactory<LGADHitCalibration_factory, eicrecon::LGADHitCalibration,
                                   LGADHitCalibrationConfig> {
private:
  ParameterRef<double> m_c_slope{this, "cSlope", config().c_slope};
  ParameterRef<double> m_c_intercept{this, "cIntercept", config().c_intercept};
  ParameterRef<double> m_t_slope{this, "tSlope", config().t_slope};
  ParameterRef<double> m_t_intercept{this, "tIntercept", config().t_intercept};
};

} // namespace eicrecon
