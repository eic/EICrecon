// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the dRICH
#pragma once

#include <CherenkovPhotonDetector.h>
#include <DD4hep/Detector.h>
#include <OpticalBoundary.h>
#include <ParametricSurface.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>

#include "IrtGeo.h"

namespace richgeo {
  class IrtGeoDRICH : public IrtGeo {

    public:
      IrtGeoDRICH(std::string compactFile_, std::shared_ptr<spdlog::logger> log_) :
        IrtGeo("DRICH",compactFile_,log_) { DD4hep_to_IRT(); }
      IrtGeoDRICH(dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_) :
        IrtGeo("DRICH",det_,log_) { DD4hep_to_IRT(); }
      ~IrtGeoDRICH();

    protected:
      void DD4hep_to_IRT() override;

    private:
      // FIXME: should be smart pointers, but IRT methods sometimes assume ownership of such raw pointers
      FlatSurface*             m_surfEntrance;
      CherenkovPhotonDetector* m_irtPhotonDetector;
      FlatSurface*             m_aerogelFlatSurface;
      FlatSurface*             m_filterFlatSurface;
      SphericalSurface*        m_mirrorSphericalSurface;
      OpticalBoundary*         m_mirrorOpticalBoundary;
      FlatSurface*             m_sensorFlatSurface;

  };
}
