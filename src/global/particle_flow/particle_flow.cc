// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4hep/Vector3f.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <vector>
#include <utility>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/particle_flow/TrackClusterSubtractor_factory.h"

extern "C" {

void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // ====================================================================
  // PFAlpha: baseline PF implementation
  // ====================================================================

  std::vector<std::pair<double, double>> thetaRanges{
      {3.0 * dd4hep::degree, 50.0 * dd4hep::degree},
      {45.0 * dd4hep::degree, 135.0 * dd4hep::degree},
      {130.0 * dd4hep::degree, 177.0 * dd4hep::degree}};

  auto subDivideByTheta =
      [&thetaRanges](const edm4eic::TrackSegment& projection) -> std::vector<int> {
    const double theta = edm4hep::utils::anglePolar(projection.getTrack().getMomentum());
    std::vector<int> binIndices;
    for (std::size_t iTheta = 0; iTheta < thetaRanges.size(); ++iTheta) {
      if ((theta >= thetaRanges[iTheta].first) && (theta < thetaRanges[iTheta].second)) {
        binIndices.push_back(iTheta);
      }
    }
    return binIndices;
  };

  app->Add(new JOmniFactoryGeneratorT<SubDivideCollection_factory<edm4eic::TrackSegment>>(
      "EndcapPBarrelEndcapNCalorimeterTrackProjections", {"CalorimeterTrackProjections"},
      {"EndcapNCalorimeterTrackProjections", "BarrelCalorimeterTrackProjections",
       "EndcapPCalorimeterTrackProjections"},
      {.function = subDivideByTheta}, app));

  // backward -----------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapNRemnantClusters",
      {"EcalEndcapNTrackClusterMatches", "EcalEndcapNClusters",
       "EndcapNCalorimeterTrackProjections"},
      {"EcalEndcapNRemnantClusters", "EcalEndcapNExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalEndcapNTrackExpectedClusterLinks"},
#else
       "EcalEndcapNTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapNRemnantClusters",
      {"HcalEndcapNTrackClusterMatches", "HcalEndcapNClusters",
       "EndcapNCalorimeterTrackProjections"},
      {"HcalEndcapNRemnantClusters", "HcalEndcapNExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalEndcapNTrackExpectedClusterLinks"},
#else
       "HcalEndcapNTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // central ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalBarrelRemnantClusters",
      {"EcalBarrelTrackClusterMatches", "EcalBarrelClusters", "BarrelCalorimeterTrackProjections"},
      {"EcalBarrelRemnantClusters", "EcalBarrelExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalBarrelTrackExpectedClusterLinks"},
#else
       "EcalBarrelTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalBarrelRemnantClusters",
      {"HcalBarrelTrackClusterMatches", "HcalBarrelClusters", "BarrelCalorimeterTrackProjections"},
      {"HcalBarrelRemnantClusters", "HcalBarrelExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalBarrelTrackExpectedClusterLink"},
#else
       "HcalBarrelTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  // forward ------------------------------------------------------------

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "EcalEndcapPRemnantClusters",
      {"EcalEndcapPTrackClusterMatches", "EcalEndcapPClusters",
       "EndcapPCalorimeterTrackProjections"},
      {"EcalEndcapPRemnantClusters", "EcalEndcapPExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalEndcapPTrackExpectedClusterLinks"},
#else
       "EcalEndcapPTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "LFHCALRemnantClusters",
      {"LFHCALTrackSplitMergeClusterMatches", "LFHCALClusters",
       "EndcapPCalorimeterTrackProjections"},
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      {"LFHCALRemnantClusters", "LFHCALExpectedClusters", "LFHCALTrackExpectedClusterLinks"},
#else
      {"LFHCALRemnantClusters", "LFHCALExpectedClusters", "LFHCALTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterSubtractor_factory>(
      "HcalEndcapPInsertRemnantClusters",
      {"HcalEndcapPInsertTrackSplitMergeClusterMatches", "HcalEndcapPInsertClusters",
       "EndcapPCalorimeterTrackProjections"},
      {"HcalEndcapPInsertRemnantClusters", "HcalEndcapPInsertExpectedClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalEndcapPInsertTrackExpectedClusterLinks"},
#else
       "HcalEndcapPInsertTrackExpectedClusterMatches"},
#endif
      {.energyFractionToSubtract = 1.0, .defaultPDG = 211, .surfaceToUse = 1},
      app // TODO: remove me once fixed
      ));
}
} // extern "C"
