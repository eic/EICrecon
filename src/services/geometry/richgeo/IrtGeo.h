// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the RICHes
#pragma once

#include <string>
#include <spdlog/spdlog.h>

// DD4Hep
#include "DD4hep/Detector.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DD4hep/DD4hepUnits.h"

// IRT
#include "IRT/CherenkovDetectorCollection.h"
#include "IRT/CherenkovPhotonDetector.h"
#include "IRT/CherenkovRadiator.h"
#include "IRT/OpticalBoundary.h"
#include "IRT/ParametricSurface.h"

// local
#include "RichGeo.h"

namespace richgeo {
  class IrtGeo {
    public:

      // constructor: creates IRT-DD4hep bindings using main `Detector` handle `*det_`
      IrtGeo(std::string detName_, dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_);
      // alternate constructor: use compact file for DD4hep geometry (backward compatibility)
      IrtGeo(std::string detName_, std::string compactFile_, std::shared_ptr<spdlog::logger> log_);
      virtual ~IrtGeo();

      // access the full IRT geometry
      CherenkovDetectorCollection *GetIrtDetectorCollection() { return m_irtDetectorCollection; }

    protected:

      // protected methods
      virtual void DD4hep_to_IRT() = 0;    // given DD4hep geometry, produce IRT geometry
      void SetReadoutIDToPositionLambda(); // define the `cell ID -> pixel position` converter, correcting to sensor surface
      void SetRefractiveIndexTable();      // fill table of refractive indices

      // inputs
      std::string m_detName;

      // DD4hep geometry handles
      dd4hep::Detector   *m_det;
      dd4hep::DetElement m_detRich;
      dd4hep::Position   m_posRich;

      // cell ID conversion
      std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;
      std::unordered_map<int,richgeo::Sensor> m_sensor_info; // sensor ID -> sensor info

      // IRT geometry handles
      CherenkovDetectorCollection *m_irtDetectorCollection;
      CherenkovDetector           *m_irtDetector;

      // logger
      std::shared_ptr<spdlog::logger> m_log;

    private:

      // set all geometry handles
      void Bind();
  };
}
