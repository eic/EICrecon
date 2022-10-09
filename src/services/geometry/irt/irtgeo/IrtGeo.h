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
#include "DD4hep/Printout.h"

// IRT
#include "CherenkovDetectorCollection.h"
#include "CherenkovPhotonDetector.h"
#include "CherenkovRadiator.h"
#include "OpticalBoundary.h"
#include "ParametricSurface.h"

class IrtGeo {
  public:

    // constructor
    IrtGeo(std::string detName_, std::string compactFile_="");
    IrtGeo(std::string detName_, dd4hep::Detector *det_) : detName(detName_), det(det_) { Bind(); }
    ~IrtGeo(); 

    // access the full IRT geometry
    CherenkovDetectorCollection *GetIrtGeometry() { return irtGeometry; }

  protected:

    // given DD4hep geometry, produce IRT geometry
    virtual void DD4hep_to_IRT() = 0;

    // inputs
    std::string detName;

    // DD4hep geometry handles
    dd4hep::Detector   *det;
    dd4hep::DetElement detRich;
    dd4hep::Position   posRich;

    // IRT geometry handles
    CherenkovDetectorCollection *irtGeometry;
    CherenkovDetector *irtDetector;

  private:

    // set all geometry handles
    void Bind();
};
