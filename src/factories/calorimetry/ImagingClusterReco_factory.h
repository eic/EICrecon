// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/calorimetry/ImagingClusterReco.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ImagingClusterReco_factory
    : public JOmniAlgorithmFactory<ImagingClusterReco_factory, eicrecon::ImagingClusterReco,
                                   ImagingClusterRecoConfig> {
private:
  ParameterRef<int> m_trackStopLayer{this, "trackStopLayer", config().trackStopLayer};
};

} // namespace eicrecon
