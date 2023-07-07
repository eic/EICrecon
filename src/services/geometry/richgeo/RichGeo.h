// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>
#include <iostream>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <edm4hep/SimTrackerHitData.h>

namespace richgeo {

  typedef decltype(edm4hep::SimTrackerHitData::cellID) CellIDType;

  // sensors
  // -----------------------------------------------------------------------
  /* keep track of information for a sensor
   */
  class Sensor {
    public:
      Sensor() {};
      ~Sensor() {};
      double            size;
      dd4hep::Position  surface_centroid;
      dd4hep::Direction surface_offset; // surface centroid = volume centroid + `surface_offset`
  };

  // logging
  // -----------------------------------------------------------------------
  /* print an error using `spdlog`, if available, otherwise use `std::cerr`
   * - this should only be called by static `richgeo` methods here, where we are not guaranteed to
   *   have an instance of `spdlog::logger` available
   */
  template <typename... VALS>
    static void PrintError(std::shared_ptr<spdlog::logger> m_log, std::string fmt_message, VALS... values) {
      auto message = fmt::format(fmt_message, values...);
      if(m_log!=nullptr)
        m_log->error(message);
      else
        std::cerr << "ERROR: " << message << std::endl;
    }

  // radiators
  // -----------------------------------------------------------------------
  /* in many places in the reconstruction, we need to track which radiator
   * we are referring to; these are common methods to enumerate them
   */
  enum radiator_enum {
    kAerogel,
    kGas,
    nRadiators
  };

  // return radiator name associated with index
  static std::string RadiatorName(int num, std::shared_ptr<spdlog::logger> m_log=nullptr) {
    if(num==kAerogel)  return "Aerogel";
    else if(num==kGas) return "Gas";
    else {
      PrintError(m_log, "unknown radiator number {}", num);
      return "UNKNOWN_RADIATOR";
    }
  }

  // return radiator index associated with name
  static int RadiatorNum(std::string name, std::shared_ptr<spdlog::logger> m_log=nullptr) {
    if(name=="Aerogel")  return kAerogel;
    else if(name=="Gas") return kGas;
    else {
      PrintError(m_log, "unknown radiator name {}", name);
      return -1;
    }
  }

  static int RadiatorNum(const char * name) {
    return RadiatorNum(std::string(name));
  }

  // search string `input` for a radiator name; return corresponding index
  static int ParseRadiatorName(std::string input, std::shared_ptr<spdlog::logger> m_log=nullptr) {
    if      (input.find("aerogel")!=std::string::npos) return kAerogel;
    else if (input.find("Aerogel")!=std::string::npos) return kAerogel;
    else if (input.find("gas")!=std::string::npos)     return kGas;
    else if (input.find("Gas")!=std::string::npos)     return kGas;
    else {
      PrintError(m_log, "failed to parse '{}' for radiator name", input);
      return -1;
    }
  }

}
