// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct CalorimeterCALOROCCalibrationConfig {
  // readout fields
  std::string readout{""};
  std::string layerField{""};
  std::string sectorField{""};

  // name of detelment or fields to find the local detector (for global->local transform)
  // if nothing is provided, the lowest level DetElement (from cellID) will be used
  std::string localDetElement{""};
  std::vector<std::string> localDetFields{};
  std::string maskPos{""};
  std::vector<std::string> maskPosFields{};

  std::string edep_to_npe_filename{""};
  std::vector<std::string> edep_to_npe_fields{""};

  enum class ProxyType { sum = 0, simpson = 1, templateFit = 2 } proxy_type = ProxyType::sum;

  bool timeWalkCor;
  bool usePulsePos;
  bool usePulseNPE;

  uint16_t highGainDR; // high gain dynamic range
  double gainRatio;    // gain ratio of high gain to low gain, it should be < 1

  std::string attenuationReferencePositionNamePos{""};
  std::string attenuationReferencePositionNameNeg{""};

  // parameters for attenuation function
  // [0] * exp(-|z_ref - z| / [1]) + (1 - [0]) * exp(-|z_ref - z| / [2])
  // specified in edm4eic::units where dimensionfull
  std::vector<double> attenuationParameters{0};

  std::vector<double> timeWalkCorrectionParameters{0};
  std::vector<double> lightSpeedParameters{0};

  // calibration parameters
  double slope     = 1 / 9.41e-2;
  double intercept = 0;
};

std::istream& operator>>(std::istream& in,
                         CalorimeterCALOROCCalibrationConfig::ProxyType& proxyType) {
  std::string s;
  in >> s;
  // stringifying the enums causes them to be converted to integers before conversion to strings
  if (s == "sum" or s == "0") {
    proxyType = CalorimeterCALOROCCalibrationConfig::ProxyType::sum;
  } else if (s == "simpson" or s == "1") {
    proxyType = CalorimeterCALOROCCalibrationConfig::ProxyType::simpson;
  } else if (s == "templateFit" or s == "2") {
    proxyType = CalorimeterCALOROCCalibrationConfig::ProxyType::templateFit;
  } else {
    in.setstate(std::ios::failbit); // Set the fail bit if the input is not valid
  }

  return in;
}
std::ostream& operator<<(std::ostream& out,
                         const CalorimeterCALOROCCalibrationConfig::ProxyType& proxyType) {
  switch (proxyType) {
  case CalorimeterCALOROCCalibrationConfig::ProxyType::sum:
    out << "sum";
    break;
  case CalorimeterCALOROCCalibrationConfig::ProxyType::simpson:
    out << "simpson";
    break;
  case CalorimeterCALOROCCalibrationConfig::ProxyType::templateFit:
    out << "templateFit";
    break;
  default:
    out.setstate(std::ios::failbit);
  }
  return out;
}

} // namespace eicrecon
