// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Wouter Deconinck

#pragma once

#include "algorithms/tracking/TrackerHitReconstruction.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class TrackerHitReconstruction_factory
    : public JOmniAlgorithmFactory<TrackerHitReconstruction_factory,
                                   eicrecon::TrackerHitReconstruction,
                                   TrackerHitReconstructionConfig> {
private:
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution};

  Service<DD4hep_service> m_geoSvc{this};
};

} // namespace eicrecon
