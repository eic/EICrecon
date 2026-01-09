// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <memory>

#include "JEventProcessorPODIO.h"
#include "JEventSourcePODIO.h"
#include "PodioRunFrame_service.h"

// Make this a JANA plugin
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new JEventSourceGeneratorT<JEventSourcePODIO>());
  app->ProvideService(std::make_shared<PodioRunFrame_service>(app));

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
