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

// ACTS
#ifdef WITH_IRTGEO_ACTS
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#endif

class IrtGeo {
  public:

    // constructor
    IrtGeo(std::string detName_, std::string compactFile_="", bool verbose_=false);
    IrtGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_=false)
      : detName(detName_), det(det_), verbose(verbose_)
    { Bind(); }
    ~IrtGeo(); 

    // radiators
    enum radiator_enum { kAerogel, kGas, nRadiators };
    static std::string RadiatorName(int num);
    static const char * RadiatorCStr(int num);
    static int RadiatorNum(std::string name);
    static int RadiatorNum(const char * name);

    // access the full IRT geometry
    CherenkovDetectorCollection *GetIrtDetectorCollection() { return irtDetectorCollection; }

#ifdef WITH_IRTGEO_ACTS
    // generate list ACTS disc surfaces, for a given radiator
    virtual std::vector<std::shared_ptr<Acts::DiscSurface>> TrackingPlanes(int radiator, int numPlanes) = 0;
#endif

  protected:

    // given DD4hep geometry, produce IRT geometry
    virtual void DD4hep_to_IRT() = 0;

    // inputs
    std::string detName;

    // DD4hep geometry handles
    dd4hep::Detector   *det;
    dd4hep::DetElement detRich;
    // dd4hep::DetElement *RadiatorDetElement(int num);
    dd4hep::Position   posRich;

    // IRT geometry handles
    CherenkovDetectorCollection *irtDetectorCollection;
    CherenkovDetector *irtDetector;

    // logging
    /* NOTE: EICrecon uses `spdlog` with a logging service; since `IrtGeo` is meant
     * to be usable independent of EICrecon, it uses a custom method `PrintLog`
     * - for compatibility with the EICrecon log service, set `IrtGeo::verbose`
     *   based on its log level to control whether `PrintLog` prints anything
     */
    bool verbose;
    template <typename... VALS> void PrintLog(std::string message, VALS... values) {
      if(verbose) fmt::print("[IrtGeo]     {}\n", fmt::format(message, values...));
    }
    template <typename... VALS> static void PrintError(std::string message, VALS... values) {
      fmt::print(stderr,"[IrtGeo]     ERROR: {}\n", fmt::format(message, values...));
    }

  private:

    // set all geometry handles
    void Bind();
};
