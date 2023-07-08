// Copyright (C) 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// helper functions for RICH readout

#pragma once

#include <string>
#include <fmt/format.h>
#include <functional>
#include <spdlog/spdlog.h>
#include <TRandomGen.h>

// DD4Hep
#include <DD4hep/Detector.h>
#include <DD4hep/DD4hepUnits.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <DDRec/CellIDPositionConverter.h>

// local
#include "RichGeo.h"

namespace richgeo {
  class ReadoutGeo {
    public:

      // constructor
      ReadoutGeo(std::string detName_, dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_);
      ~ReadoutGeo() {}

      // define cellID encoding
      CellIDType cellIDEncoding(int isec, int imod, int x, int y)
      {
	// encode cellID
	dd4hep::rec::CellID cellID_dd4hep;
	m_readoutCoder->set(cellID_dd4hep, "system", m_systemID);
	m_readoutCoder->set(cellID_dd4hep, "sector", isec);
	m_readoutCoder->set(cellID_dd4hep, "module", imod);
	m_readoutCoder->set(cellID_dd4hep, "x",      x);
	m_readoutCoder->set(cellID_dd4hep, "y",      y);
	CellIDType cellID(cellID_dd4hep); // in case DD4hep CellID type differs from EDM type
	return cellID;
	// m_log->trace("    x={:<2} y={:<2} => cellID={:#018X}", x, y, cellID);
      }

      // loop over readout pixels, executing `lambda(cellID)` on each
      void VisitAllReadoutPixels(std::function<void(CellIDType)> lambda) { m_loopCellIDs(lambda); }

      // generated k rng cell IDs, executing `lambda(cellID)` on each
      void VisitAllRngPixels(std::function<void(CellIDType)> lambda, float p) { m_rngCellIDs(lambda, p); }

// set RNG seed
void SetSeed(unsigned long seed) { m_random.SetSeed(seed); }

    protected:

      // common objects
      std::shared_ptr<spdlog::logger> m_log;
      std::string            m_detName;
      dd4hep::Detector*      m_det;
      dd4hep::DetElement     m_detRich;
      dd4hep::BitFieldCoder* m_readoutCoder;
      int                    m_systemID;
      int                    m_num_sec;
      int                    m_num_mod;
      int                    m_num_px;

      // local function to loop over cellIDs; defined in initialization and called by `VisitAllReadoutPixels`
      std::function< void(std::function<void(CellIDType)>) > m_loopCellIDs;
      // local function to generate rng cellIDs; defined in initialization and called by `VisitAllRngPixels`
      std::function< void(std::function<void(CellIDType)>, float) > m_rngCellIDs;

    private:

      // random number generators
      TRandomMixMax m_random;

  };
}
