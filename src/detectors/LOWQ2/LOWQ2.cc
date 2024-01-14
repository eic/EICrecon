// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>
#include <fmt/format.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/digi/SplitGeometry_factory.h"


extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization of silicon hits
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
         "TaggerTrackerRawHits",
         {"TaggerTrackerHits"},
         {"TaggerTrackerRawHits"},
         {
           .threshold = 1.5 * dd4hep::keV,
           .timeResolution = 2 * dd4hep::ns,
         },
         app
    ));

    // This should really be done before digitization as summing hits in the same cell couldn't evet be mixed between layers. At the moment just prep for clustering.    
    std::vector<int> moduleIDs{1,2};
    std::vector<int> layerIDs {0,1,2,4};
    std::vector<std::string> moduleDiv;
    std::vector<std::vector<std::string>> layerDiv;
    
    
    for(int mod_id : moduleIDs){
      moduleDiv.push_back(fmt::format("TaggerTrackerM{}RawHits",mod_id));
    }
      

    //std::vector<std::string> moduleDiv = {"TaggerTrackerM1RawHits","TaggerTrackerM2RawHits"};

    app->Add(new JOmniFactoryGeneratorT<SplitGeometry_factory>(
         "TaggerTrackerSplitHits",
         {"TaggerTrackerRawHits"},
         moduleDiv,
         {
	   .divisions = moduleIDs,
           .readout   = "TaggerTrackerHits",
           .division  = "module",
         },
         app
    ));

//     for(int lay_id : layerIDs){
	
//     } 



  }
}
