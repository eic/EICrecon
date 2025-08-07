// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once

#include <string>
#include <variant>
#include <iostream>

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct ImagingTopoClusterConfig {

  // maximum difference in layer numbers that can be considered as neighbours
  int neighbourLayersRange = 1;
  // maximum distance of local (x, y) to be considered as neighbors at the same layer
  std::vector<std::variant<std::string, double>> localDistXY = {1.0 * dd4hep::mm, 1.0 * dd4hep::mm};
  // maximum distance of global (eta, phi) to be considered as neighbors at different layers (if layerMode==etaphi)
  std::vector<double> layerDistEtaPhi = {0.01, 0.01};
  // maximum distance of global (x, y) to be considered as neighbors at different layers (if layerMode==xy)
  std::vector<std::variant<std::string, double>> layerDistXY = {1.0 * dd4hep::mm, 1.0 * dd4hep::mm};
  // determines how neighbors are determined for hits in different layers (using either eta and phi, or x and y)
  enum class ELayerMode { etaphi = 0, xy = 1, phiz = 2 };
  ELayerMode sameLayerMode = ELayerMode::xy;     // for ldiff =0
  ELayerMode diffLayerMode = ELayerMode::xy;     // for ldiff <= neighbourLayersRange

  // maximum global distance to be considered as neighbors in different sectors
  double sectorDist = 1.0 * dd4hep::cm;

  // minimum hit energy to participate clustering
  double minClusterHitEdep = 0.;
  // minimum cluster center energy (to be considered as a seed for cluster)
  double minClusterCenterEdep = 0.;
  // minimum cluster energy (to save this cluster)
  double minClusterEdep = 0.5 * dd4hep::MeV;
  // minimum number of hits (to save this cluster)
  std::size_t minClusterNhits = 10;
};

std::istream& operator>>(std::istream& in, ImagingTopoClusterConfig::ELayerMode& layerMode) {
  std::string s;
  in >> s;
  // stringifying the enums causes them to be converted to integers before conversion to strings
  if (s == "etaphi" or s == "0") {
    layerMode = ImagingTopoClusterConfig::ELayerMode::etaphi;
  } else if (s == "xy" or s == "1") {
    layerMode = ImagingTopoClusterConfig::ELayerMode::xy;
  } else if (s == "phiz" or s == "2") {
    layerMode = ImagingTopoClusterConfig::ELayerMode::phiz;
  } else {
    in.setstate(std::ios::failbit); // Set the fail bit if the input is not valid
  }

  return in;
}
std::ostream& operator<<(std::ostream& out, ImagingTopoClusterConfig::ELayerMode& layerMode) {
  switch (layerMode) {
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    out << "etaphi";
    break;
  case ImagingTopoClusterConfig::ELayerMode::xy:
    out << "xy";
    break;
  case ImagingTopoClusterConfig::ELayerMode::phiz:
    out << "phiz";
    break;
  default:
    out.setstate(std::ios::failbit);
  }
  return out;
}
} // namespace eicrecon
