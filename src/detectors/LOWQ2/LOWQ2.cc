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
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/meta/SubDivideFunctors.h"

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

    // Divide collection based on geometry segmentation labels
    // This should really be done before digitization as summing hits in the same cell couldn't evet be mixed between layers. At the moment just prep for clustering.
    std::string readout = "TaggerTrackerHits";
    std::vector<std::string> geometryLabels {"module","layer"};
    std::vector<int> moduleIDs{1,2};
    std::vector<int> layerIDs {0,1,2,3};
    std::vector<std::vector<long int>> geometryDivisions{};
    std::vector<std::string> geometryDivisionCollectionNames;

    for(int mod_id : moduleIDs){
      for(int lay_id : layerIDs){
        geometryDivisions.push_back({mod_id,lay_id});
        geometryDivisionCollectionNames.push_back(fmt::format("TaggerTrackerM{}L{}RawHits",mod_id,lay_id));
      }
    }
            
            
    app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::RawTrackerHit>>(
         "TaggerTrackerSplitHits",
         {"TaggerTrackerRawHits"},
         geometryDivisionCollectionNames,
         {
          .function = GeometrySplit{app,geometryDivisions,readout,geometryLabels},
         },
         app
      )
    );

    // Divide collection based on charge values
    std::vector<std::pair<long int,long int>> chargeDivisions{{0,10},{0,100},{100,200},{200,300},{200,400},{400,800},{400,1600},{1600,100000}};
    std::vector<std::string> chargeDivisionCollectionNames;

    for(auto [low,high]  : chargeDivisions){
      chargeDivisionCollectionNames.push_back(fmt::format("TaggerTrackerChargeGT{}LT{}RawHits",low,high));
    }           
    
    app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::RawTrackerHit>>(
         "TaggerTrackerSplitEnergy",
         {"TaggerTrackerRawHits"},
         chargeDivisionCollectionNames,
         {
          .function = RangeSplit<&edm4eic::RawTrackerHit::getCharge>{chargeDivisions},
         },
         app
      )
    );

  }
}
