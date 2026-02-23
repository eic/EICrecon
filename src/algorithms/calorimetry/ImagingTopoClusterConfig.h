// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once

#include <string>
#include <variant>
#include <iostream>

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct ImagingTopoClusterConfig {

  std::string readout;

  // maximum difference in layer numbers that can be considered as neighbours
  int neighbourLayersRange = 1;
  // maximum distance of local (x, y) to be considered as neighbors at same layers (if samelayerMode==xy)
  std::vector<std::variant<std::string, double>> sameLayerDistXY      = {1.0 * dd4hep::mm,
                                                                         1.0 * dd4hep::mm};
  std::vector<std::variant<std::string, double>> ScFi_sameLayerDistXY = {1.0 * dd4hep::mm,
                                                                         1.0 * dd4hep::mm};
  std::vector<std::variant<std::string, double>> Img_sameLayerDistXY  = {1.0 * dd4hep::mm,
                                                                         1.0 * dd4hep::mm};

  // maximum distance of local (x, y,z) to be considered as neighbors at same layers (if samelayerMode==xyz)
  std::vector<double> sameLayerDistXYZ = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm, 40.0 * dd4hep::mm};
  std::vector<double> ScFi_sameLayerDistXYZ = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm,
                                               40.0 * dd4hep::mm};
  std::vector<double> Img_sameLayerDistXYZ  = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm,
                                               40.0 * dd4hep::mm};

  // maximum distance of global (eta, phi) to be considered as neighbors at same layers (if samelayerMode==etaphi)
  std::vector<double> sameLayerDistEtaPhi      = {0.01, 0.01};
  std::vector<double> ScFi_sameLayerDistEtaPhi = {0.01, 0.01};
  std::vector<double> Img_sameLayerDistEtaPhi  = {0.01, 0.01};

  // maximum distance of global (t, z) to be considered as neighbors at same layers (if samelayerMode==tz)
  std::vector<double> sameLayerDistTZ      = {2.0 * dd4hep::mm, 2.0 * dd4hep::mm};
  std::vector<double> ScFi_sameLayerDistTZ = {2.0 * dd4hep::mm, 2.0 * dd4hep::mm};
  std::vector<double> Img_sameLayerDistTZ  = {2.0 * dd4hep::mm, 2.0 * dd4hep::mm};

  // maximum distance of global (x, y) to be considered as neighbors at different layers (if difflayerMode==xy)
  std::vector<std::variant<std::string, double>> diffLayerDistXY      = {1.0 * dd4hep::mm,
                                                                         1.0 * dd4hep::mm};
  std::vector<std::variant<std::string, double>> ScFi_diffLayerDistXY = {1.0 * dd4hep::mm,
                                                                         1.0 * dd4hep::mm};
  std::vector<std::variant<std::string, double>> Img_diffLayerDistXY  = {1.0 * dd4hep::mm,
                                                                         1.0 * dd4hep::mm};

  // maximum distance of global (x, y,z) to be considered as neighbors at different layers (if difflayerMode==xyz)
  std::vector<double> diffLayerDistXYZ = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm, 40.0 * dd4hep::mm};
  std::vector<double> ScFi_diffLayerDistXYZ = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm,
                                               40.0 * dd4hep::mm};
  std::vector<double> Img_diffLayerDistXYZ  = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm,
                                               40.0 * dd4hep::mm};

  // maximum distance of global (eta, phi) to be considered as neighbors at different layers (if difflayerMode==etaphi)
  std::vector<double> diffLayerDistEtaPhi      = {0.01, 0.01};
  std::vector<double> ScFi_diffLayerDistEtaPhi = {0.01, 0.01};
  std::vector<double> Img_diffLayerDistEtaPhi  = {0.01, 0.01};

  // maximum distance of global (t, z) to be considered as neighbors at different layers (if difflayerMode==tz)
  std::vector<double> diffLayerDistTZ      = {2.0 * dd4hep::mm, 2.0 * dd4hep::mm};
  std::vector<double> ScFi_diffLayerDistTZ = {2.0 * dd4hep::mm, 2.0 * dd4hep::mm};
  std::vector<double> Img_diffLayerDistTZ  = {2.0 * dd4hep::mm, 2.0 * dd4hep::mm};

  std::vector<double> cross_system_DistXYZ = {80.0 * dd4hep::mm, 80.0 * dd4hep::mm,
                                              40.0 * dd4hep::mm};

  // Layermodes
  enum class ELayerMode { etaphi = 0, xy = 1, xyz = 2, tz = 3 };

  // determines how neighbors are determined for hits in same layers (using either eta and phi, or x and y)
  ELayerMode sameLayerMode      = ELayerMode::xy; // for ldiff =0
  ELayerMode ScFi_sameLayerMode = ELayerMode::xyz;
  ELayerMode Img_sameLayerMode  = ELayerMode::tz;

  // determines how neighbors are determined for hits in different layers (using either eta and phi, or x and y)
  ELayerMode diffLayerMode      = ELayerMode::etaphi; // for ldiff <= neighbourLayersRange
  ELayerMode ScFi_diffLayerMode = ELayerMode::xyz;
  ELayerMode Img_diffLayerMode  = ELayerMode::etaphi;

  // maximum global distance to be considered as neighbors in different sectors
  double sectorDist              = 3.0 * dd4hep::cm;
  double cross_system_sectorDist = 5.0 * dd4hep::cm;
  double ScFi_sectorDist         = 5.0 * dd4hep::cm;
  double Img_sectorDist          = 3.0 * dd4hep::cm;

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
  } else if (s == "xyz" or s == "2") {
    layerMode = ImagingTopoClusterConfig::ELayerMode::xyz;
  } else if (s == "tz" or s == "3") {
    layerMode = ImagingTopoClusterConfig::ELayerMode::tz;
  } else {
    in.setstate(std::ios::failbit); // Set the fail bit if the input is not valid
  }

  return in;
}
std::ostream& operator<<(std::ostream& out, const ImagingTopoClusterConfig::ELayerMode& layerMode) {
  switch (layerMode) {
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    out << "etaphi";
    break;
  case ImagingTopoClusterConfig::ELayerMode::xy:
    out << "xy";
    break;
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    out << "xyz";
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    out << "tz";
    break;
  default:
    out.setstate(std::ios::failbit);
  }
  return out;
}
} // namespace eicrecon
