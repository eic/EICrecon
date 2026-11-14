// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024 Wouter Deconinck, Yann Bedfer

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/digi/MPGDTrackerDigi.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class MPGDTrackerDigi_factory
    : public JOmniAlgorithmFactory<MPGDTrackerDigi_factory, eicrecon::MPGDTrackerDigi,
                                   MPGDTrackerDigiConfig> {
private:
  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

  ParameterRef<double> m_threshold{this, "threshold", config().threshold};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution};
  ParameterRef<std::array<double, 2>> m_stripResolutions{
      this, "stripResolutions", config().stripResolutions, "Space resolutions for p/n strips"};
  ParameterRef<std::array<int, 2>> m_stripNumbers{this, "stripNumbers", config().stripNumbers,
                                                  "Number of p/n strips per module"};
  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};
};

} // namespace eicrecon
