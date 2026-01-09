// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplicationFwd.h>
#include <TROOT.h>
#include <string>
#include <vector>

#include "eicrecon_cli.h"

/// The default plugins
/// Add new default plugin names here and the main() will do JApplication::AddPlugin() for you.
std::vector<std::string> EICRECON_DEFAULT_PLUGINS = {
    // clang-format off
    "log",
    "dd4hep",
    "evaluator",
    "acts",
    "algorithms_init",
    "pid_lut",
    "richgeo",
    "rootfile",
    "beam",
    "reco",
    "tracking",
    "pid",
    "global_pid_lut",
    "EEMC",
    "BEMC",
    "FEMC",
    "EHCAL",
    "BHCAL",
    "FHCAL",
    "B0ECAL",
    "ZDC",
    "BTRK",
    "BVTX",
    "DIRC",
    // A unified IRT 2.0 plugin for testbed BRICH/FRICH an "real" PFRCIH/DRICH detectors;
    "RICH-IRT",
    "ECTRK",
    "MPGD",
    "B0TRK",
    "RPOTS",
    "FOFFMTRK",
    "BTOF",
    "ECTOF",
    "LOWQ2",
    "LUMISPECCAL",
    "podio",
    "janatop",
    // clang-format on
};

int main(int narg, char** argv) {
  ROOT::EnableThreadSafety();

  std::vector<std::string> default_plugins = EICRECON_DEFAULT_PLUGINS;

  auto options = jana::GetCliOptions(narg, argv, false);

  if (jana::HasPrintOnlyCliOptions(options, default_plugins)) {
    return -1;
  }

  AddAvailablePluginsToOptionParams(options, default_plugins);

  japp = jana::CreateJApplication(options);

  auto exit_code = jana::Execute(japp, options);

  delete japp;
  return exit_code;
}
