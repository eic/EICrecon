// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the dRICH
#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <IRT/CherenkovPhotonDetector.h>
#include <IRT/OpticalBoundary.h>
#include <IRT/ParametricSurface.h>
#include <TVector3.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <string>

#include "IrtGeo.h"
#include "services/geometry/richgeo/RichGeo.h"

namespace richgeo {
class IrtGeoDRICH : public IrtGeo {

public:
  IrtGeoDRICH(gsl::not_null<const dd4hep::Detector*> det_,
              gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> conv_,
              std::shared_ptr<spdlog::logger> log_)
      : IrtGeo("DRICH", det_, conv_, log_) {
    DD4hep_to_IRT();
  }
  ~IrtGeoDRICH();
  TVector3 GetSensorSurfaceNorm(CellIDType);

protected:
  void DD4hep_to_IRT() override;

private:
  // FIXME: should be smart pointers, but IRT methods sometimes assume ownership of such raw pointers
  FlatSurface* m_surfEntrance;
  CherenkovPhotonDetector* m_irtPhotonDetector;
  FlatSurface* m_aerogelFlatSurface;
  FlatSurface* m_filterFlatSurface;
  SphericalSurface* m_mirrorSphericalSurface;
  OpticalBoundary* m_mirrorOpticalBoundary;
  FlatSurface* m_sensorFlatSurface;
};
} // namespace richgeo
