// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim, Aiden Wu

#pragma once

#include <edm4eic/unit_system.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "algorithms/digi/CALOROCDigitizationConfig.h"

namespace eicrecon {

struct CalorimeterCALOROCRecoConfig {
  enum class EnergyCalibration { ADC, NpeLUT };
  EnergyCalibration energyCalibration{EnergyCalibration::NpeLUT};

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

  std::string calorocType{"1A"};
  CALOROCDigitizationConfig caloroc{};
  double adcToEnergy{1 * edm4eic::unit::GeV};
  // Visible energy = p0 + p1 * ToT + p2 * ToT squared.
  std::vector<double> totEnergyParameters{
      0 * edm4eic::unit::GeV, 1 * edm4eic::unit::GeV / edm4eic::unit::ns,
      0 * edm4eic::unit::GeV / edm4eic::unit::ns / edm4eic::unit::ns};
  // Pedestal per sample in single-gain ADC counts.
  double pedestalADC{0};
  std::string sampFrac{"1.0"};

  std::string edep_to_npe_filename{""};
  std::vector<std::string> edep_to_npe_fields{};
  enum class ProxyType { sum = 0, simpson = 1, templateFit = 2 } proxy_type = ProxyType::sum;

  bool timeWalkCor{false};

  uint16_t highGainDR{0}; // high gain dynamic range
  double gainRatio{1};    // gain ratio of low gain to high gain, it should be < 1

  std::vector<double> timeWalkCorrectionParameters{0};

  // calibration parameters: E = (npe / layer_gain) * slope + intercept
  double slope     = 1 / 9.41e-2;
  double intercept = 0;
};

inline std::istream& operator>>(std::istream& in,
                                CalorimeterCALOROCRecoConfig::ProxyType& proxyType) {
  std::string s;
  in >> s;
  // stringifying the enums causes them to be converted to integers before conversion to strings
  if (s == "sum" or s == "0") {
    proxyType = CalorimeterCALOROCRecoConfig::ProxyType::sum;
  } else if (s == "simpson" or s == "1") {
    proxyType = CalorimeterCALOROCRecoConfig::ProxyType::simpson;
  } else if (s == "templateFit" or s == "2") {
    proxyType = CalorimeterCALOROCRecoConfig::ProxyType::templateFit;
  } else {
    in.setstate(std::ios::failbit); // Set the fail bit if the input is not valid
  }

  return in;
}
inline std::ostream& operator<<(std::ostream& out,
                                const CalorimeterCALOROCRecoConfig::ProxyType& proxyType) {
  switch (proxyType) {
  case CalorimeterCALOROCRecoConfig::ProxyType::sum:
    out << "sum";
    break;
  case CalorimeterCALOROCRecoConfig::ProxyType::simpson:
    out << "simpson";
    break;
  case CalorimeterCALOROCRecoConfig::ProxyType::templateFit:
    out << "templateFit";
    break;
  default:
    out.setstate(std::ios::failbit);
  }
  return out;
}

} // namespace eicrecon
