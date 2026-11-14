// Created by Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/Measurement2DCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackerMeasurementFromHits.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class TrackerMeasurementFromHits_factory
    : public JOmniAlgorithmFactory<TrackerMeasurementFromHits_factory,
                                   eicrecon::TrackerMeasurementFromHits, NoConfig> {
private:
  Service<DD4hep_service> m_DD4hepSvc{this};
  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
};

} // namespace eicrecon
