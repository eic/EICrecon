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
      virtual ~IrtGeo();

      // access the full IRT geometry
      CherenkovDetectorCollection *GetIrtDetectorCollection() { return m_irtDetectorCollection; }

    protected:

      // given DD4hep geometry, produce IRT geometry
      virtual void DD4hep_to_IRT() = 0;

      // inputs
      std::string m_detName;

      // DD4hep geometry handles
      dd4hep::Detector   *m_det;
      dd4hep::DetElement m_detRich;
      dd4hep::Position   m_posRich;

      // IRT geometry handles
      CherenkovDetectorCollection *m_irtDetectorCollection;
      CherenkovDetector *m_irtDetector;

      // logger
      Logger& m_log;

    private:

      // set all geometry handles
      void Bind();
  };
}
