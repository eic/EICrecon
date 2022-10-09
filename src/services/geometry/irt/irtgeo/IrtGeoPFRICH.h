// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

// bind IRT and DD4hep geometries for the pfRICH
#pragma once

#include "IrtGeo.h"

class IrtGeoPFRICH : public IrtGeo {

  public:
    IrtGeoPFRICH(std::string compactFile_="") : IrtGeo("PFRICH",compactFile_) { DD4hep_to_IRT(); }
    IrtGeoPFRICH(dd4hep::Detector *det_)      : IrtGeo("PFRICH",det_)         { DD4hep_to_IRT(); }
    ~IrtGeoPFRICH() {}

  protected:
    void DD4hep_to_IRT() override;
};
