// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

// bind IRT and DD4hep geometries for the RICHes
#pragma once

#include <string>
#include <fmt/format.h>

// DD4Hep
#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"

// ACTS
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>

// local
#include "RichGeo.h"

namespace rich {
  class ActsGeo {
    public:

      // constructor
      ActsGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_=false)
        : m_detName(detName_), m_det(det_), m_log(Logger::Instance(verbose_))
      {
        // capitalize m_detName
        std::transform(m_detName.begin(), m_detName.end(), m_detName.begin(), ::toupper);
      }
      ~ActsGeo(); 

      // generate list ACTS disc surfaces, for a given radiator
      std::vector<std::shared_ptr<Acts::DiscSurface>> TrackingPlanes(int radiator, int numPlanes);

    protected:

      std::string m_detName;
      dd4hep::Detector *m_det;
      Logger& m_log;

    private:

  };
}
