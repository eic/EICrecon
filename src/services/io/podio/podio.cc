// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "JEventSourcePODIO.h"
#include "EDM4hepWriter.h"


// Make this a JANA plugin
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventSourceGeneratorT<JEventSourcePODIO>());

    // Only add a EDM4hepWriter if the user has specified a PODIO:OUTPUT_FILE or PODIO:OUTPUT_COLLECTION_NAMES
    if( app->GetJParameterManager()->Exists("PODIO:OUTPUT_FILE") || app->GetJParameterManager()->Exists("PODIO:OUTPUT_COLLECTION_NAMES")){
        app->Add(new EDM4hepWriter());
    }
}
}

