// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the RICHes
#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/DetectorData.h>
#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <unordered_map>

// local
#include "RichGeo.h"

namespace richgeo {
class IrtGeo {
public:
  // constructor: creates IRT-DD4hep bindings using main `Detector` handle `*det_`
  IrtGeo(std::string detName_, gsl::not_null<const dd4hep::Detector*> det_,
         gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> conv_,
         std::shared_ptr<spdlog::logger> log_);
  virtual ~IrtGeo();

  // access the full IRT geometry
  CherenkovDetectorCollection* GetIrtDetectorCollection() const { return m_irtDetectorCollection; }

protected:
  // protected methods
  virtual void DD4hep_to_IRT() = 0; // given DD4hep geometry, produce IRT geometry
  void
  SetReadoutIDToPositionLambda(); // define the `cell ID -> pixel position` converter, correcting to sensor surface
  void SetRefractiveIndexTable(); // fill table of refractive indices
  // read `VariantParameters` for a vector
  template <class VecT>
  VecT GetVectorFromVariantParameters(dd4hep::rec::VariantParameters* pars, std::string key) const {
    return VecT(pars->get<double>(key + "_x"), pars->get<double>(key + "_y"),
                pars->get<double>(key + "_z"));
  }

  // inputs
  std::string m_detName;

  // DD4hep geometry handles
  gsl::not_null<const dd4hep::Detector*> m_det;
  dd4hep::DetElement m_detRich;
  dd4hep::Position m_posRich;

  // cell ID conversion
  gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> m_converter;
  std::unordered_map<int, richgeo::Sensor> m_sensor_info; // sensor ID -> sensor info

  // IRT geometry handles
  CherenkovDetectorCollection* m_irtDetectorCollection{};
  CherenkovDetector* m_irtDetector{};

  // logger
  std::shared_ptr<spdlog::logger> m_log;

private:
  // set all geometry handles
  void Bind();
};
} // namespace richgeo
