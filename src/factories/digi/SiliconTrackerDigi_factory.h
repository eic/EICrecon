// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/digi/SiliconTrackerDigi.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class SiliconTrackerDigi_factory
    : public JOmniAlgorithmFactory<SiliconTrackerDigi_factory, eicrecon::SiliconTrackerDigi,
                                   SiliconTrackerDigiConfig> {
private:
  ParameterRef<double> m_threshold{this, "threshold", config().threshold};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution};
  ParameterRef<double> m_cellThreshold{this, "cellThreshold", config().cellThreshold};
};

} // namespace eicrecon
