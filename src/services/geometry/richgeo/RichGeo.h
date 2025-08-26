// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>
#include <iostream>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <edm4hep/SimTrackerHitData.h>

namespace richgeo {

using CellIDType = decltype(edm4hep::SimTrackerHitData::cellID);

// sensors
// -----------------------------------------------------------------------
/* keep track of information for a sensor
   */
class Sensor {
public:
  Sensor() {};
  ~Sensor() {};
  double size;
  dd4hep::Position surface_centroid;
  dd4hep::Direction surface_offset; // surface centroid = volume centroid + `surface_offset`
};

// radiators
// -----------------------------------------------------------------------
/* in many places in the reconstruction, we need to track which radiator
   * we are referring to; these are common methods to enumerate them
   */
enum radiator_enum { kAerogel, kGas, nRadiators };

// return radiator name associated with index
[[maybe_unused]] static std::string RadiatorName(int num,
                                                 std::shared_ptr<spdlog::logger> m_log = nullptr) {
  if (num == kAerogel)
    return "Aerogel";
  else if (num == kGas)
    return "Gas";
  else {
    if (m_log)
      m_log->error("unknown radiator number {}", num);
    else
      std::cerr << "ERROR: unknown radiator number " << num << std::endl;
    return "UNKNOWN_RADIATOR";
  }
}

// return radiator index associated with name
[[maybe_unused]] static int RadiatorNum(std::string name,
                                        std::shared_ptr<spdlog::logger> m_log = nullptr) {
  if (name == "Aerogel")
    return kAerogel;
  else if (name == "Gas")
    return kGas;
  else {
    if (m_log)
      m_log->error("unknown radiator name {}", name);
    else
      std::cerr << "ERROR: unknown radiator name " << name << std::endl;
    return -1;
  }
}

[[maybe_unused]] static int RadiatorNum(const char* name,
                                        std::shared_ptr<spdlog::logger> m_log = nullptr) {
  return RadiatorNum(std::string(name), m_log);
}

// search string `input` for a radiator name; return corresponding index
[[maybe_unused]] static int ParseRadiatorName(std::string input,
                                              std::shared_ptr<spdlog::logger> m_log = nullptr) {
  if (input.find("aerogel") != std::string::npos)
    return kAerogel;
  else if (input.find("Aerogel") != std::string::npos)
    return kAerogel;
  else if (input.find("gas") != std::string::npos)
    return kGas;
  else if (input.find("Gas") != std::string::npos)
    return kGas;
  else {
    if (m_log)
      m_log->error("failed to parse '{}' for radiator name", input);
    else
      std::cerr << "ERROR: failed to parse '" << input << "' for radiator name" << std::endl;
    return -1;
  }
}

} // namespace richgeo
