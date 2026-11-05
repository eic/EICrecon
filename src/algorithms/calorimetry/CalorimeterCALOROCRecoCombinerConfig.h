// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct CalorimeterCALOROCRecoCombinerConfig {
  // readout used to decode the CellID (needed to find the local DetElement)
  std::string readout{""};

  // name of detelment or fields to find the local detector (for global->local transform)
  // if nothing is provided, the lowest level DetElement (from cellID) will be used
  std::string localDetElement{""};
  std::vector<std::string> localDetFields{};

  // take z from the (truth) position of the NpeHit instead of the time difference
  bool useNpeHitPos{false};

  // names of the constants (in the geometry) that give the z position of the two SiPMs
  std::string attenuationReferencePositionNamePos{""};
  std::string attenuationReferencePositionNameNeg{""};

  // parameters for attenuation function
  // [0] * exp(-|z_ref - z| / [1]) + (1 - [0]) * exp(-|z_ref - z| / [2])
  // specified in edm4eic::units where dimensionfull
  std::vector<double> attenuationParameters{0};

  // z = (t_N - t_P) * [0] + [1]
  std::vector<double> lightSpeedParameters{0};
};

} // namespace eicrecon
