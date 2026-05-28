// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#include "SecondaryVertexFinder.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/VoidNavigator.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/AnnealingUtility.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveGridTrackDensity.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/IVertexFinder.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/LinearizedTrack.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <edm4eic/Cov4f.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector4f.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "ActsGeometryProvider.h"
#include "SecondaryVertexFinderConfig.h"
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

// Type aliases for Acts vertexing components (implementation detail)
using ImpactPointEstimator = Acts::ImpactPointEstimator;
using Linearizer           = Acts::HelicalTrackLinearizer;
using VertexFitter         = Acts::AdaptiveMultiVertexFitter;
using VertexFinder         = Acts::AdaptiveMultiVertexFinder;
using VertexFinderOptions  = Acts::VertexingOptions;
using SeedFinder           = Acts::AdaptiveGridDensityVertexFinder;

void SecondaryVertexFinder::storeVertices(
    const std::vector<Acts::Vertex>& vertices,
    const edm4eic::ReconstructedParticleCollection& reconParticles,
    edm4eic::VertexCollection& outputVertices, int vertexType) const {
  for (const auto& vtx : vertices) {
    edm4eic::Cov4f cov(vtx.fullCovariance()(0, 0), vtx.fullCovariance()(1, 1),
                       vtx.fullCovariance()(2, 2), vtx.fullCovariance()(3, 3),
                       vtx.fullCovariance()(0, 1), vtx.fullCovariance()(0, 2),
                       vtx.fullCovariance()(0, 3), vtx.fullCovariance()(1, 2),
                       vtx.fullCovariance()(1, 3), vtx.fullCovariance()(2, 3));
    auto eicvertex = outputVertices.create();
    eicvertex.setType(vertexType);
    eicvertex.setChi2(static_cast<float>(vtx.fitQuality().first));
    eicvertex.setNdf(static_cast<float>(vtx.fitQuality().second));
    eicvertex.setPosition({
        static_cast<float>(vtx.position().x()),
        static_cast<float>(vtx.position().y()),
        static_cast<float>(vtx.position().z()),
        static_cast<float>(vtx.time()),
    });
    eicvertex.setPositionError(cov);

    for (const auto& t : vtx.tracks()) {
      const auto par = Acts::InputTrack::extractParameters(t.originalParams);
      trace("Track local position from vertex = {} mm, {} mm",
            par.localPosition().x() / Acts::UnitConstants::mm,
            par.localPosition().y() / Acts::UnitConstants::mm);
      float loc_a = par.localPosition().x();
      float loc_b = par.localPosition().y();

      for (const auto& part : reconParticles) {
        const auto& tracks = part.getTracks();
        for (const auto& trk : tracks) {
          const auto& traj    = trk.getTrajectory();
          const auto& trkPars = traj.getTrackParameters();
          for (const auto& trkPar : trkPars) {
            double EPSILON = std::numeric_limits<double>::epsilon();
            if (std::abs((trkPar.getLoc().a / edm4eic::unit::mm) -
                         (loc_a / Acts::UnitConstants::mm)) < EPSILON &&
                std::abs((trkPar.getLoc().b / edm4eic::unit::mm) -
                         (loc_b / Acts::UnitConstants::mm)) < EPSILON) {
              trace("From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
                    trkPar.getLoc().a / edm4eic::unit::mm, trkPar.getLoc().b / edm4eic::unit::mm);
              eicvertex.addToAssociatedParticles(part);
            }
          }
        }
      }
    }
    debug("One AMVF vertex found at (x,y,z) = ({}, {}, {}) mm.",
          vtx.position().x() / Acts::UnitConstants::mm,
          vtx.position().y() / Acts::UnitConstants::mm,
          vtx.position().z() / Acts::UnitConstants::mm);
  }
}

void SecondaryVertexFinder::process(const SecondaryVertexFinder::Input& input,
                                    const SecondaryVertexFinder::Output& output) const {
  auto [recotracks, trackStates, tracks] = input;
  auto [outputVertices]                  = output;

  if (tracks->size_impl() == 0) {
    debug("No tracks in the container - skipping");
    return;
  }

  // Convert algorithm log level to Acts log level
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("AMVF", acts_level));

  // Geometry and field contexts
  const auto& gctx = m_geoSvc->getActsGeometryContext();
  const auto& mctx = m_geoSvc->getActsMagneticFieldContext();

  Acts::EigenStepper<> stepper(m_BField);

  // Set up propagator with void navigator
  using Propagator = Acts::Propagator<Acts::EigenStepper<>>;
  auto propagator  = std::make_shared<Propagator>(stepper, Acts::VoidNavigator{},
                                                  logger().cloneWithSuffix("Prop"));

  // Set up track density used during vertex seeding
  Acts::AdaptiveGridTrackDensity::Config trkDensityConfig;
  trkDensityConfig.spatialBinExtent  = m_cfg.spatialBinExtent;
  trkDensityConfig.temporalBinExtent = m_cfg.temporalBinExtent;
  trkDensityConfig.useTime           = m_cfg.useTime;
  Acts::AdaptiveGridTrackDensity trkDensity(trkDensityConfig);

  // Setup the track linearizer
  Linearizer::Config linearizerConfig(m_BField, propagator);
  std::unique_ptr<const Acts::Logger> linearizer_log = logger().cloneWithSuffix("Linearizer");
  Linearizer linearizer(linearizerConfig, std::move(linearizer_log));

  // Set up deterministic annealing with user-defined temperatures
  Acts::AnnealingUtility::Config annealingConfig;
  annealingConfig.setOfTemperatures = {9., 1.0};
  Acts::AnnealingUtility annealingUtility(annealingConfig);

  // Setup the vertex fitter
  ImpactPointEstimator::Config ipEstConfig(m_BField, propagator);
  ImpactPointEstimator ipEst(ipEstConfig);
  VertexFitter::Config vertexFitterConfig(ipEst);

  vertexFitterConfig.annealingTool     = annealingUtility;
  vertexFitterConfig.minWeight         = m_cfg.minWeight;
  vertexFitterConfig.maxDistToLinPoint = m_cfg.maxDistToLinPoint;
  vertexFitterConfig.doSmoothing       = m_cfg.doSmoothing;
  vertexFitterConfig.useTime           = m_cfg.useTime;
  vertexFitterConfig.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterConfig.trackLinearizer.connect<&Linearizer::linearizeTrack>(&linearizer);
  VertexFitter vertexFitter(std::move(vertexFitterConfig));

  // Set up vertex seeder and finder
  SeedFinder::Config seederConfig(trkDensity);
  seederConfig.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  auto seeder = std::make_shared<SeedFinder>(SeedFinder::Config{seederConfig});

  VertexFinder::Config vertexfinderConfig(std::move(vertexFitter), std::move(seeder),
                                          std::move(ipEst), m_BField);

  vertexfinderConfig.initialVariances           = m_cfg.initialVariances;
  vertexfinderConfig.useTime                    = m_cfg.useTime;
  vertexfinderConfig.useSeedConstraint          = m_cfg.useSeedConstraint;
  vertexfinderConfig.tracksMaxZinterval         = m_cfg.tracksMaxZinterval;
  vertexfinderConfig.maxIterations              = m_cfg.maxIterations;
  vertexfinderConfig.doFullSplitting            = m_cfg.doFullSplitting;
  vertexfinderConfig.tracksMaxSignificance      = m_cfg.tracksMaxSignificance;
  vertexfinderConfig.maxMergeVertexSignificance = m_cfg.maxMergeVertexSignificance;

  if (m_cfg.useTime) {
    // When using time, we have an extra contribution to the chi2 by the time
    // coordinate.
    vertexfinderConfig.tracksMaxSignificance      = m_cfg.tracksMaxSignificance;
    vertexfinderConfig.maxMergeVertexSignificance = m_cfg.maxMergeVertexSignificance;
  }

  vertexfinderConfig.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexfinderConfig.bField = m_BField;

  // Build the ACTS track container
  auto trackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(*trackStates);
  auto trackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>(*tracks);
  ActsExamples::ConstTrackContainer constTracks(trackContainer, trackStateContainer);

  // Build BoundTrackParameters for all tracks upfront
  std::vector<Acts::BoundTrackParameters> allTrackParameters;
  allTrackParameters.reserve(constTracks.size());
  for (const auto& track : constTracks) {
    allTrackParameters.emplace_back(track.referenceSurface().getSharedPtr(), track.parameters(),
                                    track.covariance(), track.particleHypothesis());
    trace("Track local position at input = {} mm, {} mm",
          track.parameters()[Acts::eBoundLoc0] / Acts::UnitConstants::mm,
          track.parameters()[Acts::eBoundLoc1] / Acts::UnitConstants::mm);
  }

  // Vertex type: 1 for primary, 0 for secondary
  const int vertexType = m_cfg.isPrimary ? 1 : 0;

  if (m_cfg.isPrimary) {
    // Primary vertex mode: run AMVF on all tracks at once
    VertexFinder finder(std::move(vertexfinderConfig));
    auto state = finder.makeState(mctx);
    VertexFinderOptions vfOptions(gctx, mctx);

    std::vector<Acts::InputTrack> inputTracks;
    for (auto& tp : allTrackParameters) {
      inputTracks.emplace_back(&tp);
    }

    std::vector<Acts::Vertex> vertices;
    auto result = finder.find(inputTracks, vfOptions, state);
    if (result.ok()) {
      vertices = std::move(result.value());
    }

    storeVertices(vertices, *recotracks, *outputVertices, vertexType);
  } else {
    // Secondary vertex mode: run AMVF on all pairwise track combinations
    VertexFinder finder(std::move(vertexfinderConfig));
    auto state = finder.makeState(mctx);
    VertexFinderOptions vfOptions(gctx, mctx);

    std::vector<Acts::InputTrack> inputTracks;
    for (unsigned int i = 0; i < allTrackParameters.size() - 1; i++) {
      for (unsigned int j = i + 1; j < allTrackParameters.size(); j++) {
        inputTracks.emplace_back(&allTrackParameters[i]);
        inputTracks.emplace_back(&allTrackParameters[j]);

        std::vector<Acts::Vertex> verticesSec;
        auto resultSec = finder.find(inputTracks, vfOptions, state);
        if (resultSec.ok()) {
          verticesSec = std::move(resultSec.value());
        }

        storeVertices(verticesSec, *recotracks, *outputVertices, vertexType);

        inputTracks.clear();
      }
    }
  }
}

} // namespace eicrecon
