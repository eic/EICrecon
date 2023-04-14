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

        "log",
        "dd4hep",
        "acts",
        "richgeo",
        "rootfile",
        "algorithms_calorimetry",
        "algorithms_tracking",
        "algorithms_digi",
        "digi",
        "reco",
        "tracking",
        "pid",
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
        "DRICH",
        "ECTRK",
        "MPGD",
        "B0TRK",
        "RPOTS",
        "BTOF",
        "ECTOF",
        "podio",
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
