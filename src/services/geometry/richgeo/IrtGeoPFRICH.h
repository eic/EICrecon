// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the pfRICH
#pragma once

#include "IrtGeo.h"

namespace richgeo {
  class IrtGeoPFRICH : public IrtGeo {

    public:
      IrtGeoPFRICH(std::string compactFile_, std::shared_ptr<spdlog::logger> log_) :
        IrtGeo("PFRICH",compactFile_,log_) { DD4hep_to_IRT(); }
      IrtGeoPFRICH(dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_) :
        IrtGeo("PFRICH",det_,log_) { DD4hep_to_IRT(); }
      ~IrtGeoPFRICH();

    protected:
      void DD4hep_to_IRT() override;

    private:
      // FIXME: should be smart pointers, but IRT methods sometimes assume ownership of such raw pointers
      FlatSurface*             m_surfEntrance;
      CherenkovPhotonDetector* m_irtPhotonDetector;
      FlatSurface*             m_aerogelFlatSurface;
      FlatSurface*             m_filterFlatSurface;
      FlatSurface*             m_sensorFlatSurface;
  };
}
