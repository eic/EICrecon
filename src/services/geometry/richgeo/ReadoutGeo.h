// Copyright (C) 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// helper functions for RICH readout

#pragma once

#include <DD4hep/DetElement.h>
// DD4Hep
#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Parsers/Primitives.h>
#include <TRandomGen.h>
#include <spdlog/logger.h>
#include <functional>
#include <gsl/pointers>
#include <memory>
#include <string>

// local
#include "RichGeo.h"

namespace richgeo {
class ReadoutGeo {
public:
  // constructor
  ReadoutGeo(std::string detName_, std::string readoutClass_,
             gsl::not_null<const dd4hep::Detector*> det_,
             gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> conv_,
             std::shared_ptr<spdlog::logger> log_);
  ~ReadoutGeo() {}

  // define cellID encoding
  CellIDType cellIDEncoding(int isec, int ipdu, int isipm, int x, int y) {
    // encode cellID
    dd4hep::rec::CellID cellID_dd4hep;
    m_readoutCoder->set(cellID_dd4hep, "system", m_systemID);
    m_readoutCoder->set(cellID_dd4hep, "sector", isec);
    m_readoutCoder->set(cellID_dd4hep, "pdu", ipdu);
    m_readoutCoder->set(cellID_dd4hep, "sipm", isipm);
    m_readoutCoder->set(cellID_dd4hep, "x", x);
    m_readoutCoder->set(cellID_dd4hep, "y", y);
    CellIDType cellID(cellID_dd4hep); // in case DD4hep CellID type differs from EDM type
    return cellID;
    // m_log->trace("    x={:<2} y={:<2} => cellID={:#018X}", x, y, cellID);
  }

  // loop over readout pixels, executing `lambda(cellID)` on each
  void VisitAllReadoutPixels(std::function<void(CellIDType)> lambda) { m_loopCellIDs(lambda); }

  // generated k rng cell IDs, executing `lambda(cellID)` on each
  void VisitAllRngPixels(std::function<void(CellIDType)> lambda, float p) {
    m_rngCellIDs(lambda, p);
  }

  // pixel gap mask
  bool PixelGapMask(CellIDType cellID, dd4hep::Position pos_hit_global) const;

  // transform global position `pos` to sensor `id` frame position
  // IMPORTANT NOTE: this has only been tested for the dRICH; if you use it, test it carefully...
  dd4hep::Position GetSensorLocalPosition(CellIDType id, dd4hep::Position pos) const;

  // set RNG seed
  void SetSeed(unsigned long seed) { m_random.SetSeed(seed); }

protected:
  // common objects
  std::shared_ptr<spdlog::logger> m_log;
  std::string m_detName;
  std::string m_readoutClass;
  gsl::not_null<const dd4hep::Detector*> m_det;
  gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> m_conv;
  dd4hep::DetElement m_detRich;
  dd4hep::BitFieldCoder* m_readoutCoder;
  int m_systemID;
  int m_num_sec;
  int m_num_pdus;
  int m_num_sipms_per_pdu;
  int m_num_px;
  double m_pixel_size;

  // local function to loop over cellIDs; defined in initialization and called by `VisitAllReadoutPixels`
  std::function<void(std::function<void(CellIDType)>)> m_loopCellIDs;
  // local function to generate rng cellIDs; defined in initialization and called by `VisitAllRngPixels`
  std::function<void(std::function<void(CellIDType)>, float)> m_rngCellIDs;

private:
  // random number generators
  TRandomMixMax m_random;
};
} // namespace richgeo
