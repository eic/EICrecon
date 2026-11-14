// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2026 Wouter Deconinck, Yann Bedfer

#pragma once

#include "algorithms/tracking/MPGDHitReconstruction.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class MPGDHitReconstruction_factory
    : public JOmniAlgorithmFactory<MPGDHitReconstruction_factory, eicrecon::MPGDHitReconstruction,
                                   MPGDHitReconstructionConfig> {
private:
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution};

  Service<DD4hep_service> m_geoSvc{this};
};

} // namespace eicrecon
