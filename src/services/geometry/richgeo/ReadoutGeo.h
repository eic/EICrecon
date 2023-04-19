// Copyright (C) 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// helper functions for RICH readout

#pragma once

#include <string>
#include <fmt/format.h>
#include <functional>
#include <TRandomGen.h>

// DD4Hep
#include <DD4hep/Detector.h>
#include <DD4hep/DD4hepUnits.h>
#include <DDSegmentation/BitFieldCoder.h>

// local
#include "RichGeo.h"

namespace richgeo {
  class ReadoutGeo {
    public:

      // constructor
      ReadoutGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_=false);
      ~ReadoutGeo() {}

      // random number generators
      TRandomMixMax m_random;

      // loop over readout pixels, executing `lambda(cellID)` on each
      void VisitAllReadoutPixels(std::function<void(uint64_t)> lambda) { m_loopCellIDs(lambda); }

      // generated k rng cell IDs, executing `lambda(cellID)` on each
      void VisitAllRngPixels(std::function<void(uint64_t)> lambda, float p) { m_rngCellIDs(lambda, p); }


    protected:

      // common objects
      Logger&                m_log;
      std::string            m_detName;
      dd4hep::Detector*      m_det;
      dd4hep::DetElement     m_detRich;
      dd4hep::BitFieldCoder* m_readoutCoder;
      int                    m_systemID;
      int                    m_num_sec;
      int                    m_num_mod;
      int                    m_num_px;

      // local function to loop over cellIDs; defined in initialization and called by `VisitAllReadoutPixels`
      std::function< void(std::function<void(uint64_t)>) > m_loopCellIDs;
      // local function to generate rng cellIDs; defined in initialization and called by `VisitAllRngPixels`
      std::function< void(std::function<void(uint64_t)>, float) > m_rngCellIDs;

    private:

  };
}
