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

namespace rich {
  class IrtGeo {
    public:

      // constructor: creates IRT-DD4hep bindings using main `Detector` handle `*det_`
      IrtGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_=false);
      // alternate constructor: use compact file for DD4hep geometry (backward compatibility)
      IrtGeo(std::string detName_, std::string compactFile_="", bool verbose_=false);
      ~IrtGeo(); 

      // access the full IRT geometry
      CherenkovDetectorCollection *GetIrtDetectorCollection() { return m_irtDetectorCollection; }

      // cell ID -> position converter
      dd4hep::Position CellID_to_Position(dd4hep::DDSegmentation::CellID cell_id);

    protected:

      // protected methods
      virtual void DD4hep_to_IRT() = 0; // given DD4hep geometry, produce IRT geometry
      void SetRefractiveIndexTable();   // fill table of refractive indices

      // inputs
      std::string m_detName;

      // DD4hep geometry handles
      dd4hep::Detector   *m_det;
      dd4hep::DetElement m_detRich;
      dd4hep::Position   m_posRich;

      // cell ID conversion
      std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;
      // - pixel surface centroid = pixel volume centroid + `m_sensor_surface_offset`:
      std::unordered_map<int,dd4hep::Direction> m_sensor_surface_offset; // sensor id -> offset

      // IRT geometry handles
      CherenkovDetectorCollection *m_irtDetectorCollection;
      CherenkovDetector           *m_irtDetector;

      // logger
      Logger& m_log;

    private:

      // set all geometry handles
      void Bind();
  };
}
