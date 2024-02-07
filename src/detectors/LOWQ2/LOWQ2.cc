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
    std::vector<int> layerIDs {0,1,2,3};
    std::vector<std::vector<int>> segmentIDs{};
    std::vector<std::string> segmentDiv;

    for(int mod_id : moduleIDs){
      for(int lay_id : layerIDs){
        segmentIDs.push_back({mod_id,lay_id});
        segmentDiv.push_back(fmt::format("TaggerTrackerM{}L{}RawHits",mod_id,lay_id));
      }
    }

    app->Add(new JOmniFactoryGeneratorT<SplitGeometry_factory<edm4eic::RawTrackerHit>>(
         "TaggerTrackerSplitHits",
         {"TaggerTrackerRawHits"},
         segmentDiv,
         {
           .divisions = segmentIDs,
           .readout   = "TaggerTrackerHits",
           .division  = {"module","layer"},
         },
         app
      )
    );

  }
}
