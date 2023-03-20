// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

// bind IRT and DD4hep geometries for the RICHes
#pragma once

#include <string>
#include <functional>
#include <fmt/format.h>

// DD4Hep
#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"

// ACTS
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>

// local
#include "RichGeo.h"

namespace richgeo {
  class ActsGeo {
    public:

      // constructor and destructor
      ActsGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_=false);
      ~ActsGeo() {}

      // generate list ACTS disc surfaces, for a given radiator
      std::vector<std::shared_ptr<Acts::Surface>> TrackingPlanes(int radiator, int numPlanes);

      // lambdas to tell us if a point is within a radiator
      std::function<bool(double,double,double)> WithinRadiator[nRadiators];

    protected:

      std::string m_detName;
      dd4hep::Detector *m_det;
      Logger& m_log;

    private:

  };
}
