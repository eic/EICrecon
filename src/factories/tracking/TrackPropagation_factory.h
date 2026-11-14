// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tyler Kutz, Wouter Deconinck

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class TrackPropagation_factory
    : public JOmniAlgorithmFactory<TrackPropagation_factory, eicrecon::TrackPropagation,
                                   TrackPropagationConfig> {
private:
  Service<DD4hep_service> m_GeoSvc{this};
  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
};

} // namespace eicrecon
