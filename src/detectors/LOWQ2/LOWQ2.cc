// Copyright 2023-2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/TrackSegment.h>
#include <edm4eic/unit_system.h>
#include <fmt/core.h>
#include <math.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/FarDetectorLinearProjection_factory.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factory.h"
#include "factories/fardetectors/FarDetectorMLReconstruction_factory.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    std::string tracker_readout = "TaggerTrackerHits";

    // Digitization of silicon hits
    app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
         "TaggerTrackerRawHits",
         {
           "TaggerTrackerHits"
         },
         {
           "TaggerTrackerRawHits",
           "TaggerTrackerRawHitAssociations"
         },
         {
           .threshold = 1.5 * dd4hep::keV,
           .timeResolution = 2 * dd4hep::ns,
         },
         app
    ));

    // Divide collection based on geometry segmentation labels
    // This should really be done before digitization as summing hits in the same cell couldn't even be mixed between layers. At the moment just prep for clustering.
    std::string readout = "TaggerTrackerHits";
    std::vector<std::string> geometryLabels {"module","layer"};
    std::vector<int> moduleIDs{1,2};
    std::vector<int> layerIDs {0,1,2,3};
    std::vector<std::vector<long int>> geometryDivisions{};
    std::vector<std::string> geometryDivisionCollectionNames;
    std::vector<std::string> outputClusterCollectionNames;
    std::vector<std::string> outputTrackTags;
    std::vector<std::vector<std::string>> moduleClusterTags;

    for(int mod_id : moduleIDs){
      outputTrackTags.push_back(fmt::format("TaggerTrackerM{}Tracks",mod_id));
      moduleClusterTags.push_back({});
      for(int lay_id : layerIDs){
        geometryDivisions.push_back({mod_id,lay_id});
        geometryDivisionCollectionNames.push_back(fmt::format("TaggerTrackerM{}L{}RawHits",mod_id,lay_id));
        outputClusterCollectionNames.push_back(fmt::format("TaggerTrackerM{}L{}ClusterPositions",mod_id,lay_id));
        moduleClusterTags.back().push_back(outputClusterCollectionNames.back());
      }
    }

    app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::RawTrackerHit>>(
         "TaggerTrackerSplitHits",
         {"TaggerTrackerRawHits"},
         geometryDivisionCollectionNames,
         {
          .function = GeometrySplit{geometryDivisions,readout,geometryLabels},
         },
         app
      )
    );


    app->Add(new JOmniFactoryGeneratorT<FarDetectorTrackerCluster_factory>(
        "TaggerTrackerClustering",
        geometryDivisionCollectionNames,
        outputClusterCollectionNames,
        {
          .readout = "TaggerTrackerHits",
          .x_field  = "x",
          .y_field  = "y",
          .hit_time_limit = 10 * edm4eic::unit::ns,
        },
        app
    ));

    // Linear tracking for each module, loop over modules
    for(int i=0; i<moduleIDs.size(); i++){
      std::string outputTrackTag = outputTrackTags[i];
      std::vector<std::string> inputClusterTags = moduleClusterTags[i];

      app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearTracking_factory>(
          outputTrackTag,
          inputClusterTags,
          {outputTrackTag},
          {
            .layer_hits_max = 100,
            .chi2_max = 0.001,
            .n_layer = 4,
            .restrict_direction = true,
            .optimum_theta = -M_PI+0.026,
            .optimum_phi = 0,
            .step_angle_tolerance = 0.05,
          },
          app
      ));
    }

    // Combine the tracks from each module into one collection
    app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::TrackSegment>>(
         "TaggerTrackerTrackSegments",
         outputTrackTags,
         {"TaggerTrackerTrackSegments"},
         app
      )
    );

    // Project tracks onto a plane
    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearProjection_factory>(
         "TaggerTrackerProjectedTracks",
         {"TaggerTrackerTrackSegments"},
         {"TaggerTrackerProjectedTracks"},
         {
           .plane_position = {0.0,0.0,0.0},
           .plane_a = {0.0,1.0,0.0},
           .plane_b = {0.0,0.0,1.0},
         },
         app
    ));

    // Vector reconstruction at origin
    app->Add(new JOmniFactoryGeneratorT<FarDetectorMLReconstruction_factory>(
        "TaggerTrackerTrajectories",
        {"TaggerTrackerProjectedTracks","MCBeamElectrons"},
        {"TaggerTrackerTrajectories","TaggerTrackerTrackParameters","TaggerTrackerTracks"},
        {
          .modelPath = "calibrations/tmva/LowQ2_DNN_CPU.weights.xml",
          .methodName = "DNN_CPU",
        },
        app
    ));

  }
}
