// Copyright 2023-2025, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackParticleAssociation.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/unit_system.h>
#include <fmt/core.h>
#include <fmt/format.h> // IWYU pragma: keep
#include <cmath>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/meta/SubDivideFunctors.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/PulseNoise_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconPulseGeneration_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/FarDetectorLinearProjection_factory.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/fardetectors/FarDetectorTransportationPostML_factory.h"
#include "factories/fardetectors/FarDetectorTransportationPreML_factory.h"
#endif
#include "factories/fardetectors/FarDetectorMLReconstruction_factory.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/meta/ONNXInference_factory.h"
#endif

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;
  using eicrecon::JOmniFactoryGeneratorT;

  std::string readout = "TaggerTrackerHits";

  app->Add(new JOmniFactoryGeneratorT<SiliconChargeSharing_factory>(
      "TaggerTrackerChargeSharing", {"TaggerTrackerHits"}, {"TaggerTrackerSharedHits"},
      {
          .sigma_sharingx = 15 * dd4hep::um,
          .sigma_sharingy = 15 * dd4hep::um,
          .min_edep       = 0.1 * edm4eic::unit::keV,
          .readout        = readout,
      },
      app));
  //  Generate signal pulse from hits
  app->Add(new JOmniFactoryGeneratorT<SiliconPulseGeneration_factory>(
      "TaggerTrackerPulseGeneration", {"TaggerTrackerSharedHits"}, {"TaggerTrackerHitPulses"},
      {
          .pulse_shape_function = "LandauPulse",
          .pulse_shape_params   = {1.0, 2 * edm4eic::unit::ns},
          .ignore_thres         = 15.0e-8,
          .timestep             = 0.2 * edm4eic::unit::ns,
      },
      app));

  // Combine pulses into larger pulses
  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      "TaggerTrackerPulseCombiner", {"TaggerTrackerHitPulses"}, {"TaggerTrackerCombinedPulses"},
      {
          .minimum_separation = 25 * edm4eic::unit::ns,
      },
      app));

  // Add noise to pulses
  app->Add(new JOmniFactoryGeneratorT<PulseNoise_factory>(
      "TaggerTrackerPulseNoise", {"EventHeader", "TaggerTrackerCombinedPulses"},
      {"TaggerTrackerCombinedPulsesWithNoise"},
      {
          .poles    = 5,
          .variance = 1.0,
          .alpha    = 0.5,
          .scale    = 0.000002,
      },
      app));

  // Digitization of silicon hits
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "TaggerTrackerRawHits", {"EventHeader", "TaggerTrackerHits"},
      {"TaggerTrackerRawHits", "TaggerTrackerRawHitAssociations"},
      {
          .threshold      = 1.5 * edm4eic::unit::keV,
          .timeResolution = 2 * edm4eic::unit::ns,
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "TaggerTrackerRecHits", {"TaggerTrackerRawHits"}, {"TaggerTrackerRecHits"},
      {
          .timeResolution = 2,
      },
      app));

  // Divide collection based on geometry segmentation labels
  // This should really be done before digitization as summing hits in the same cell couldn't even be mixed between layers. At the moment just prep for clustering.
  std::vector<std::string> geometryLabels{"module", "layer"};
  std::vector<int> moduleIDs{1, 2};
  std::vector<int> layerIDs{0, 1, 2, 3};
  std::vector<std::vector<long int>> geometryDivisions{};
  std::vector<std::string> geometryDivisionCollectionNames;
  std::vector<std::string> outputClusterCollectionNames;
  std::vector<std::string> outputTrackTags;
  std::vector<std::string> outputTrackAssociationTags;
  std::vector<std::vector<std::string>> moduleClusterTags;

  for (int mod_id : moduleIDs) {
    outputTrackTags.push_back(fmt::format("TaggerTrackerM{}LocalTracks", mod_id));
    outputTrackAssociationTags.push_back(
        fmt::format("TaggerTrackerM{}LocalTrackAssociations", mod_id));
    moduleClusterTags.emplace_back();
    for (int lay_id : layerIDs) {
      geometryDivisions.push_back({mod_id, lay_id});
      geometryDivisionCollectionNames.push_back(
          fmt::format("TaggerTrackerM{}L{}RecHits", mod_id, lay_id));
      outputClusterCollectionNames.push_back(
          fmt::format("TaggerTrackerM{}L{}ClusterPositions", mod_id, lay_id));
      moduleClusterTags.back().push_back(outputClusterCollectionNames.back());
    }
  }

  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::TrackerHit>>(
      "TaggerTrackerSplitHits", {"TaggerTrackerRecHits"}, geometryDivisionCollectionNames,
      {
          .function = GeometrySplit{geometryDivisions, readout, geometryLabels},
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<FarDetectorTrackerCluster_factory>(
      "TaggerTrackerClustering", geometryDivisionCollectionNames, outputClusterCollectionNames,
      {
          .readout        = "TaggerTrackerHits",
          .x_field        = "x",
          .y_field        = "y",
          .hit_time_limit = 10 * edm4eic::unit::ns,
      },
      app));

  // Linear tracking for each module, loop over modules
  for (std::size_t i = 0; i < moduleIDs.size(); i++) {
    std::string outputTrackTag                = outputTrackTags[i];
    std::string outputTrackAssociationTag     = outputTrackAssociationTags[i];
    std::vector<std::string> inputClusterTags = moduleClusterTags[i];

    inputClusterTags.emplace_back("TaggerTrackerRawHitAssociations");

    app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearTracking_factory>(
        outputTrackTag, {inputClusterTags}, {outputTrackTag, outputTrackAssociationTag},
        {
            .layer_hits_max       = 200,
            .chi2_max             = 0.001,
            .n_layer              = 4,
            .restrict_direction   = true,
            .optimum_theta        = -M_PI + 0.026,
            .optimum_phi          = 0,
            .step_angle_tolerance = 0.05,
        },
        app));
  }

  // Combine the tracks from each module into one collection
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::Track, true>>(
      "TaggerTrackerLocalTracks", outputTrackTags, {"TaggerTrackerLocalTracks"}, app));

  // Combine the associations from each module into one collection
  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      "TaggerTrackerLocalTrackAssociations", outputTrackAssociationTags,
      {"TaggerTrackerLocalTrackAssociations"}, app));

  // Project tracks onto a plane
  app->Add(new JOmniFactoryGeneratorT<FarDetectorLinearProjection_factory>(
      "TaggerTrackerProjectedTracks", {"TaggerTrackerLocalTracks"},
      {"TaggerTrackerProjectedTracks"},
      {
          .plane_position = {0.0, 0.0, 0.0},
          .plane_a        = {0.0, 1.0, 0.0},
          .plane_b        = {0.0, 0.0, 1.0},
      },
      app));

#if EDM4EIC_VERSION_MAJOR >= 8
  app->Add(new JOmniFactoryGeneratorT<FarDetectorTransportationPreML_factory>(
      "TaggerTrackerTransportationPreML",
      {"TaggerTrackerProjectedTracks", "MCScatteredElectrons", "MCBeamElectrons"},
      {"TaggerTrackerFeatureTensor", "TaggerTrackerTargetTensor"},
      {
          .beamE = 10.0,
      },
      app));
  app->Add(new JOmniFactoryGeneratorT<ONNXInference_factory>(
      "TaggerTrackerTransportationInference", {"TaggerTrackerFeatureTensor"},
      {"TaggerTrackerPredictionTensor"},
      {
          .modelPath = "calibrations/onnx/TaggerTrackerTransportation.onnx",
      },
      app));
  app->Add(new JOmniFactoryGeneratorT<FarDetectorTransportationPostML_factory>(
      "TaggerTrackerTransportationPostML", {"TaggerTrackerPredictionTensor", "MCBeamElectrons"},
      {"TaggerTrackerReconstructedParticles"},
      {
          .beamE = 10.0,
      },
      app));
#endif

  // Vector reconstruction at origin
  app->Add(new JOmniFactoryGeneratorT<FarDetectorMLReconstruction_factory>(
      "TaggerTrackerTrajectories",
      {"TaggerTrackerProjectedTracks", "MCBeamElectrons", "TaggerTrackerLocalTracks",
       "TaggerTrackerLocalTrackAssociations"},
      {"TaggerTrackerTrajectories", "TaggerTrackerTrackParameters", "TaggerTrackerTracks",
       "TaggerTrackerTrackAssociations"},
      {
          .modelPath  = "calibrations/tmva/LowQ2_DNN_CPU.weights.xml",
          .methodName = "DNN_CPU",
      },
      app));
}
}
