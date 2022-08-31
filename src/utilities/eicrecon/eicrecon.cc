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
        std::cout << "        histsfile file.root    Create a root file for histograms/trees produced by plugins" << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        return -1;
    }
    if (options.flags[jana::ShowVersion]) {
        // Show version information and exit immediately
        jana::PrintVersion();
        return -1;
    }

    // Optionally create root file
    // TODO: This needs to be done by a service that plugins can use to get the TDirectory from to avoid conflicts with podio files
    TFile *rootfile = nullptr;
    if (options.params.count("histsfile")){
        LOG << "Creating root file: " << options.params["histsfile"] << " for histograms/trees from plugins" << LOG_END;
        rootfile = new TFile(options.params["histsfile"].c_str(), "RECREATE", "eicrecon plugin histograms/trees");
    }

    japp = jana::CreateJApplication(options);

    if(const char* env_p = std::getenv("EICrecon_MY")) japp->AddPluginPath( std::string(env_p) + "/plugins" );

    japp->AddPlugin( "podio"           );
    japp->AddPlugin( "dd4hep"          );
    japp->AddPlugin( "log"          );
    // japp->AddPlugin( "calorimetry"     );
    // japp->AddPlugin( "tracking"        );
    japp->AddPlugin( "BEMC"            );

    auto exit_code = jana::Execute(japp, options);

    // If root file was created, then close it
    if( rootfile ){
        LOG << "Closing histsfile: " << rootfile->GetName() << LOG_END;
        rootfile->Write();
        rootfile->Close();
        delete rootfile;
    }

    delete japp;
    return exit_code;
}
