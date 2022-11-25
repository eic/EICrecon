// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

// bind IRT and DD4hep geometries for the pfRICH
#pragma once

#include "IrtGeo.h"

class IrtGeoPFRICH : public IrtGeo {

  public:
    IrtGeoPFRICH(std::string compactFile_="", bool verbose_=false) : IrtGeo("PFRICH",compactFile_,verbose_) { DD4hep_to_IRT(); }
    IrtGeoPFRICH(dd4hep::Detector *det_,      bool verbose_=false) : IrtGeo("PFRICH",det_,verbose_)         { DD4hep_to_IRT(); }
    ~IrtGeoPFRICH() {}

#ifdef WITH_IRTGEO_ACTS
    std::vector<std::shared_ptr<Acts::DiscSurface>> TrackingPlanes(int radiator, int numPlanes) override;
#endif

  protected:
    void DD4hep_to_IRT() override;
};
