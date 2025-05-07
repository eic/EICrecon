// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the pfRICH
#pragma once

#include <gsl/pointers>
#include <memory>

#include "IrtGeo.h"

class CherenkovPhotonDetector;
class FlatSurface;
namespace dd4hep::rec {
class CellIDPositionConverter;
}
namespace dd4hep {
class Detector;
}
namespace spdlog {
class logger;
}

namespace richgeo {
class IrtGeoPFRICH : public IrtGeo {

public:
  IrtGeoPFRICH(gsl::not_null<const dd4hep::Detector*> det_,
               gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> conv_,
               std::shared_ptr<spdlog::logger> log_)
      : IrtGeo("PFRICH", det_, conv_, log_) {
    DD4hep_to_IRT();
  }
  ~IrtGeoPFRICH();

protected:
  void DD4hep_to_IRT() override;

private:
  // FIXME: should be smart pointers, but IRT methods sometimes assume ownership of such raw pointers
  FlatSurface* m_surfEntrance;
  CherenkovPhotonDetector* m_irtPhotonDetector;
  FlatSurface* m_aerogelFlatSurface;
  FlatSurface* m_filterFlatSurface;
  FlatSurface* m_sensorFlatSurface;
};
} // namespace richgeo
