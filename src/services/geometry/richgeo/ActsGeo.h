// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the RICHes
#pragma once

#include <functional>
#include <gsl/gsl>
#include <spdlog/spdlog.h>
#include <string>

// DD4Hep
#include <DD4hep/Detector.h>
#include <DD4hep/DD4hepUnits.h>

// data model
#include <edm4eic/TrackPoint.h>

// local
#include "RichGeo.h"

// Forward declarations
namespace Acts {
  class Surface;
}

namespace richgeo {
  class ActsGeo {
    public:

      // constructor and destructor
      ActsGeo(std::string detName_, gsl::not_null<const dd4hep::Detector*> det_, std::shared_ptr<spdlog::logger> log_);
      ~ActsGeo() {}

      // generate list ACTS disc surfaces, for a given radiator
      std::vector<std::shared_ptr<Acts::Surface>> TrackingPlanes(int radiator, int numPlanes);

      // generate a cut to remove any track points that should not be used
      std::function<bool(edm4eic::TrackPoint)> TrackPointCut(int radiator);

    protected:

      std::string                     m_detName;
      gsl::not_null<const dd4hep::Detector*> m_det;
      std::shared_ptr<spdlog::logger> m_log;

    private:

  };
}
