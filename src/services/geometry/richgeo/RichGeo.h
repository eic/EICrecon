// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#pragma once

#include <string>
#include <fmt/format.h>

namespace richgeo {

  // local logging singleton
  /* NOTE: EICrecon uses `spdlog` with a logging service; since `RichGeo` is meant
   * to be usable independent of EICrecon, it uses a custom method `PrintLog`
   * - for compatibility with the EICrecon log service, set `verbose`
   *   based on its log level to control whether `PrintLog` prints anything
   * - static `PrintError` prints an error regardless of `verbose`
   */
  class Logger {
    private:
      Logger(bool verbose_) : verbose(verbose_) {};
    public:
      static Logger& Instance(bool verbose_) {
        static Logger thisInstance(verbose_);
        return thisInstance;
      }
      template <typename... VALS>
        void PrintLog(std::string message, VALS... values) {
          if(verbose) fmt::print("[RichGeo]     {}\n", fmt::format(message, values...));
        }
      template <typename... VALS>
        static void PrintError(std::string message, VALS... values) {
          fmt::print(stderr,"[RichGeo]     ERROR: {}\n", fmt::format(message, values...));
        }
    private:
      bool verbose;
  };

  // radiators
  /* in many places in the reconstruction, we need to track which radiator
   * we are referring to; these are common methods to enumerate them
   */
  enum radiator_enum {
    kAerogel,
    kGas,
    nRadiators
  };
  // return radiator name associated with index
  static std::string RadiatorName(int num) {
    if(num==kAerogel)  return "Aerogel";
    else if(num==kGas) return "Gas";
    else {
      Logger::PrintError("unknown radiator number {}",num);
      return "UNKNOWN_RADIATOR";
    }
  }
  // return radiator index associated with name
  static int RadiatorNum(std::string name) {
    if(name=="Aerogel")  return kAerogel;
    else if(name=="Gas") return kGas;
    else {
      Logger::PrintError("unknown radiator name {}",name);
      return -1;
    }
  }
  static int RadiatorNum(const char * name) {
    return RadiatorNum(std::string(name));
  }
  // search string `input` for a radiator name; return corresponding index
  static int ParseRadiatorName(std::string input) {
    if      (input.find("aerogel")!=std::string::npos) return kAerogel;
    else if (input.find("Aerogel")!=std::string::npos) return kAerogel;
    else if (input.find("gas")!=std::string::npos)     return kGas;
    else if (input.find("Gas")!=std::string::npos)     return kGas;
    else {
      Logger::PrintError("failed to parse '{}' for radiator name",input);
      return -1;
    }
  }

  // static dd4hep::DetElement *RadiatorDetElement(int num) {
  //   auto name = RadiatorName(num) + "_de";
  //   std::transform(
  //       name.begin(), name.end(), name.begin(),
  //       [] (auto c) {return std::tolower(c); }
  //       );
  //   for(auto const& [de_name, det_elem] : detRich.children())
  //     if(de_name.find(name)!=std::string::npos)
  //       return &det_elem;
  //   PrintError("cannot find DetElement {}",name);
  //   return nullptr;
  // }

}
