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
        "BEMC",
        "BTRK",
        "BVTX",
        "ECTRK",
        "EEMC",
        "MPGD",
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

//    /// @note: the default plugins and the plugins at $EICrecon_MY are not managed by the JComponentManager,
//    /// thus they will not be shown with the "eicrecon -c" option.
//    // Add the plugins at $EICrecon_MY/plugins
//    if(const char* env_p = std::getenv("EICrecon_MY"))
//        japp->AddPluginPath( std::string(env_p) + "/plugins" );
//    // Add the default plugins
//    jana::AddDefaultPluginsToJApplication(japp, default_plugins);

    auto exit_code = jana::Execute(japp, options);

    delete japp;
    return exit_code;
}
