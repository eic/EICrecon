// SPDX-License-Identifier: JSA
// Copyright (C) 2022 David Lawrence

#include <JANA/JApplication.h>
#include <JANA/JEventSourceGeneratorT.h>

#include "JEventProcessorPODIO.h"
#include "JEventSourcePODIO.h"
#include "JEventSourcePODIOLegacy.h"


// Make this a JANA plugin
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventSourceGeneratorT<JEventSourcePODIO>());
    app->Add(new JEventSourceGeneratorT<JEventSourcePODIOLegacy>());

    // Disable this behavior for now so one can run eicrecon with only the
    // input file as an argument.
    // Only add a EICRootWriter if the user has specified a configuration parameter relevant to writing
//    if( app->GetJParameterManager()->Exists("podio:output_file")
//        ||  app->GetJParameterManager()->Exists("podio:output_file_copy_dir")
//        ||  app->GetJParameterManager()->Exists("podio:output_include_collections")
//        ||  app->GetJParameterManager()->Exists("podio:output_exclude_collections")        ){
        app->Add(new JEventProcessorPODIO());
//    }
}
}
