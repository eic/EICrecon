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
#include "CherenkovDetectorCollection.h"
#include "CherenkovPhotonDetector.h"
#include "CherenkovRadiator.h"
#include "OpticalBoundary.h"
#include "ParametricSurface.h"

class IrtGeo {
  public:

    // constructor
    IrtGeo(std::string detName_, std::string compactFile_="", bool verbose_=false);
    IrtGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_=false)
      : detName(detName_), det(det_), verbose(verbose_)
    { Bind(); }
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

    // logging
    /* NOTE: EICrecon uses `spdlog` with a logging service; since `IrtGeo` is meant
     * to be usable independent of EICrecon, it uses a custom method `PrintLog`
     * - for compatibility with the EICrecon log service, set `IrtGeo::verbose`
     *   based on its log level to control whether `PrintLog` prints anything
     */
    bool verbose;
    template <typename... VALS> void PrintLog(std::FILE *stream, std::string message, VALS... values) {
      if(verbose or stream==stderr)
        fmt::print(stream, "[IrtGeo]     {}\n", fmt::format(message, values...));
    }
    template <typename... VALS> void PrintLog(std::string message, VALS... values) {
      PrintLog(stdout, message, values...);
    }

  private:

    // set all geometry handles
    void Bind();
};
