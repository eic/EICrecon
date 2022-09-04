// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <iostream>

#include <TFile.h>

#include <JANA/CLI/JMain.h>

int main( int narg, char **argv)
{
    auto options = jana::ParseCommandLineOptions(narg, argv, false);

    if (options.flags[jana::ShowUsage]) {
        // Show usage information and exit immediately
        jana::PrintUsage();
        std::cout << std::endl << "-----------" << std::endl;
        std::cout << "    eicrecon parameters: (specify with -Pparam=value)" << std::endl;
        std::cout << std::endl;
        std::cout << "        -Phistsfile=file.root    Set name for histograms/trees produced by plugins" << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        return -1;
    }
    if (options.flags[jana::ShowVersion]) {
        // Show version information and exit immediately
        jana::PrintVersion();
        return -1;
    }

    japp = jana::CreateJApplication(options);

    if(const char* env_p = std::getenv("EICrecon_MY")) japp->AddPluginPath( std::string(env_p) + "/plugins" );

    japp->AddPlugin( "podio"           );
    japp->AddPlugin( "dd4hep"          );
    japp->AddPlugin( "log"             );
    japp->AddPlugin( "rootfile"        );
    // japp->AddPlugin( "calorimetry"     );
    // japp->AddPlugin( "tracking"        );
    japp->AddPlugin( "BEMC"            );
    japp->AddPlugin( "EEMC"            );

    auto exit_code = jana::Execute(japp, options);

    delete japp;
    return exit_code;
}
