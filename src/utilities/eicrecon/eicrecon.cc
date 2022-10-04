// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <iostream>

#include <TFile.h>

#include "eicrecon_cli.h"

/// The default plugins
/// Add new default plugin names here and the main() will do JApplication::AddPlugin() for you.
std::vector<std::string> EICRECON_DEFAULT_PLUGINS = {
        "podio",
        "dd4hep",
        "acts",
        "log",
        "rootfile",
        "algorithms_calorimetry",
        "algorithms_tracking",
        "algorithms_digi",
        "digi",
        "BEMC",
        "HCAL",
        "ZDC",
        "BTRK",
        "BVTX",
        "ECTRK",
        "EEMC",
        "MPGD",
        "RPOTS",
        "tracking"
};

int main( int narg, char **argv)
{
    std::vector<std::string> default_plugins = EICRECON_DEFAULT_PLUGINS;

    auto options = jana::GetCliOptions(narg, argv, false);

    if (jana::HasPrintOnlyCliOptions(options, default_plugins))
        return -1;

    AddAvailablePluginsToOptionParams(options, default_plugins);

    japp = jana::CreateJApplication(options);

    auto exit_code = jana::Execute(japp, options);

    delete japp;
    return exit_code;
}
