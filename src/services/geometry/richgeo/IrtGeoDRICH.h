// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// bind IRT and DD4hep geometries for the dRICH
#pragma once

#include "IrtGeo.h"

namespace richgeo {
  class IrtGeoDRICH : public IrtGeo {

    public:
      IrtGeoDRICH(std::string compactFile_, std::shared_ptr<spdlog::logger> log_) :
        IrtGeo("DRICH",compactFile_,log_) { DD4hep_to_IRT(); }
      IrtGeoDRICH(dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_) :
        IrtGeo("DRICH",det_,log_) { DD4hep_to_IRT(); }
      ~IrtGeoDRICH() {}

    protected:
      void DD4hep_to_IRT() override;
  };
}
