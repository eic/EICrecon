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
    std::vector<int> layerIDs {0,1,2,3};
    std::vector<std::vector<long int>> segmentIDs{};
    std::vector<std::string> segmentDiv;

    for(int mod_id : moduleIDs){
      for(int lay_id : layerIDs){
        segmentIDs.push_back({mod_id,lay_id});
        segmentDiv.push_back(fmt::format("TaggerTrackerM{}L{}RawHits",mod_id,lay_id));
      }
    }
            
    auto id_dec = app->GetService<DD4hep_service>()->detector()->readout("TaggerTrackerHits").idSpec().decoder();
    std::vector<size_t> div {id_dec->index("module"),id_dec->index("layer")};

    // Lambda function which selects which collections to insert the hit into
    auto splitHits = [id_dec,segmentIDs,div](const edm4eic::RawTrackerHit& hit){
      auto cellID = hit.getCellID();
      std::vector<long int> ids {id_dec->get(cellID, div[0]),id_dec->get(cellID, div[1])};
      auto index = std::find(segmentIDs.begin(),segmentIDs.end(),ids);
      if(index != segmentIDs.end()){
        return std::vector<int>{index-segmentIDs.begin()};      
      } else {
        return std::vector<int>{};
      }
    };

    app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::RawTrackerHit>>(
         "TaggerTrackerSplitHits",
         {"TaggerTrackerRawHits"},
         segmentDiv,
         {
          .function = splitHits,
         },
         app
      )
    );

    std::vector<std::pair<long int,long int>> energyIDs{{0,10},{0,100},{100,200},{200,300},{200,400},{400,800},{400,1600},{1600,100000}};
    std::vector<std::string> energyDiv;

    for(auto [low,high]  : energyIDs){
      energyDiv.push_back(fmt::format("TaggerTrackerChargeGT{}LT{}RawHits",low,high));
    }           

    // Lambda function which selects which collections to insert the hit into
    auto splitEnergy = [energyIDs](const edm4eic::RawTrackerHit& hit){
      auto charge = hit.getCharge();
      std::vector<int> ids;
      // Loop over energy ranges and find the ones that the charge falls into
      for(size_t i = 0; i < energyIDs.size(); i++){
        if(charge > energyIDs[i].first && charge < energyIDs[i].second){
          ids.push_back(i);
        }
      }
      return ids;
    };
    
    app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::RawTrackerHit>>(
         "TaggerTrackerSplitEnergy",
         {"TaggerTrackerRawHits"},
         energyDiv,
         {
          .function = splitEnergy,
         },
         app
      )
    );

  }
}
