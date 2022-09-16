// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <iostream>

#include <TFile.h>

#include "eicrecon_cli.h"

/// The default plugins
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

    japp = jana::CreateJApplication(options);

    if(const char* env_p = std::getenv("EICrecon_MY")) japp->AddPluginPath( std::string(env_p) + "/plugins" );

    // TODO: add by command line paras
    for( auto plugin : default_plugins) japp->AddPlugin( plugin );
//    japp->AddPlugin( "podio"           );
//    japp->AddPlugin( "dd4hep"          );
//    japp->AddPlugin( "acts"        );
//    japp->AddPlugin( "log"             );
//    japp->AddPlugin( "rootfile"        );
//    japp->AddPlugin( "algorithms_calorimetry");
//    japp->AddPlugin( "algorithms_tracking");
//    japp->AddPlugin( "algorithms_digi" );
//    japp->AddPlugin( "BEMC"            );
//    japp->AddPlugin( "EEMC"            );

    auto exit_code = jana::Execute(japp, options);

    delete japp;
    return exit_code;
}
