// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <iostream>

#include <TFile.h>

#include <JANA/CLI/JMain.h>
#include <JANA/JFactoryGenerator.h>

#include <services/io/podio/JEventSourcePODIO.h>
#include <detectors/BEMC/JFactory_BEMCRawCalorimeterHit.h>

int main( int narg, char **argv)
{
    auto options = jana::ParseCommandLineOptions(narg, argv, false);

    if (options.flags[jana::ShowUsage]) {
        // Show usage information and exit immediately
        jana::PrintUsage();
        return -1;
    }
    if (options.flags[jana::ShowVersion]) {
        // Show version information and exit immediately
        jana::PrintVersion();
        return -1;
    }

    japp = jana::CreateJApplication(options);

    japp->Add( new JEventSourceGeneratorT<JEventSourcePODIO>() );
    japp->Add( new JFactoryGeneratorT<JFactory_BEMCRawCalorimeterHit>() );

    auto exit_code = jana::Execute(japp, options);
    delete japp;
    return exit_code;
}
