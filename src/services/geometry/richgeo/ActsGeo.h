// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the RICHes
#pragma once

#include <DD4hep/Detector.h>
#include <edm4eic/TrackPoint.h>
#include <spdlog/logger.h>
#include <functional>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/tracking/TrackPropagationConfig.h"

namespace richgeo {
class ActsGeo {
public:
  // constructor and destructor
  ActsGeo(std::string detName_, gsl::not_null<const dd4hep::Detector*> det_,
          std::shared_ptr<spdlog::logger> log_);
  ~ActsGeo() {}

  // generate list ACTS disc surfaces, for a given radiator
  std::vector<eicrecon::SurfaceConfig> TrackingPlanes(int radiator, int numPlanes) const;

  // generate a cut to remove any track points that should not be used
  std::function<bool(edm4eic::TrackPoint)> TrackPointCut(int radiator) const;

protected:
  std::string m_detName;
  gsl::not_null<const dd4hep::Detector*> m_det;
  std::shared_ptr<spdlog::logger> m_log;

private:
};
} // namespace richgeo
