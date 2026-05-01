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
#include <Acts/Vertexing/AdaptiveGridTrackDensity.hpp>
#include <Acts/Vertexing/IVertexFinder.hpp>
#include <Acts/Vertexing/LinearizedTrack.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
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

void SecondaryVertexFinder::process(const SecondaryVertexFinder::Input& input,
                                    const SecondaryVertexFinder::Output& output) const {
  auto [recotracks, trackStates, tracks] = input;
  auto [primaryVertices, outputVertices] = output;

  Acts::EigenStepper<> stepperSec(m_BField);

  //Need to make sure that the track container is not actually empty
  if (tracks->size_impl() > 0) {
    // Calculate primary vertex using AMVF
    calculatePrimaryVertex(*recotracks, trackStates, tracks, stepperSec, *primaryVertices);
    // Primary vertex collection container to be used in Sec. Vertex fitting
    calculateSecondaryVertex(*recotracks, trackStates, tracks, stepperSec, *outputVertices);
  }
}

//Quickly calculate the PV using the Adaptive Multi-vertex Finder
void SecondaryVertexFinder::calculatePrimaryVertex(
    const edm4eic::ReconstructedParticleCollection& reconParticles,
    const Acts::ConstVectorMultiTrajectory* trackStates,
    const Acts::ConstVectorTrackContainer* tracks, Acts::EigenStepper<> stepperSec,
    edm4eic::VertexCollection& prmVertices) const {
  // Convert algorithm log level to Acts log level
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("AMVF", acts_level));

  // Geometry and field contexts
  const auto& m_geoctx   = m_geoSvc->getActsGeometryContext();
  const auto& m_fieldctx = m_geoSvc->getActsMagneticFieldContext();

  // Set-up the propagator
  using PropagatorSec = Acts::Propagator<Acts::EigenStepper<>>;

  // Set up propagator with void navigator
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::VoidNavigator{},
                                                       logger().cloneWithSuffix("Prop"));

  // Set up track density used during vertex seeding
  Acts::AdaptiveGridTrackDensity::Config trkDensityConfig;
  // Bin extent in z-direction
  trkDensityConfig.spatialBinExtent = m_cfg.spatialBinExtent;
  // Bin extent in t-direction
  trkDensityConfig.temporalBinExtent = m_cfg.temporalBinExtent;
  trkDensityConfig.useTime           = m_cfg.useTime;
  Acts::AdaptiveGridTrackDensity trkDensity(trkDensityConfig);

  // Setup the track linearizer
  LinearizerSec::Config linearizerConfigSec(m_BField, propagatorSec);
  std::unique_ptr<const Acts::Logger> linearizer_log = logger().cloneWithSuffix("Linearizer");
  LinearizerSec linearizerSec(linearizerConfigSec, std::move(linearizer_log));

  //Staring multivertex fitter
  // Set up deterministic annealing with user-defined temperatures
  Acts::AnnealingUtility::Config annealingConfig;
  annealingConfig.setOfTemperatures = {9., 1.0};
  Acts::AnnealingUtility annealingUtility(annealingConfig);
  // Setup the vertex fitter
  ImpactPointEstimator::Config ipEstConfig(m_BField, propagatorSec);
  ImpactPointEstimator ipEst(ipEstConfig);
  VertexFitterSec::Config vertexFitterConfigSec(ipEst);

  vertexFitterConfigSec.annealingTool     = annealingUtility;
  vertexFitterConfigSec.minWeight         = m_cfg.minWeight;
  vertexFitterConfigSec.maxDistToLinPoint = m_cfg.maxDistToLinPoint;
  vertexFitterConfigSec.doSmoothing       = m_cfg.doSmoothing;
  vertexFitterConfigSec.useTime           = m_cfg.useTime;
  vertexFitterConfigSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterConfigSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);
  VertexFitterSec vertexFitterSec(std::move(vertexFitterConfigSec));

  // Set up vertex seeder and finder
  using seedFinder = Acts::AdaptiveGridDensityVertexFinder;
  std::shared_ptr<const Acts::IVertexFinder> seeder;
  seedFinder::Config seederConfig(trkDensity);
  seederConfig.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  seeder = std::make_shared<seedFinder>(seedFinder::Config{seederConfig});

  VertexFinderSec::Config vertexfinderConfigSec(std::move(vertexFitterSec), std::move(seeder),
                                                std::move(ipEst), m_BField);

  // The vertex finder state
  // Set the initial variance of the 4D vertex position. Since time is on a
  // numerical scale, we have to provide a greater value in the corresponding
  // dimension.
  vertexfinderConfigSec.initialVariances = m_cfg.initialVariances;
  //Use time for Sec. Vertex
  vertexfinderConfigSec.useTime                    = m_cfg.useTime;
  vertexfinderConfigSec.tracksMaxZinterval         = m_cfg.tracksMaxZinterval;
  vertexfinderConfigSec.maxIterations              = m_cfg.maxIterations;
  vertexfinderConfigSec.doFullSplitting            = m_cfg.doFullSplitting;
  vertexfinderConfigSec.tracksMaxSignificance      = m_cfg.tracksMaxSignificance;
  vertexfinderConfigSec.maxMergeVertexSignificance = m_cfg.maxMergeVertexSignificance;

  if (m_cfg.useTime) {
    // When using time, we have an extra contribution to the chi2 by the time
    // coordinate.
    vertexfinderConfigSec.tracksMaxSignificance      = m_cfg.tracksMaxSignificance;
    vertexfinderConfigSec.maxMergeVertexSignificance = m_cfg.maxMergeVertexSignificance;
  }

  vertexfinderConfigSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();

  vertexfinderConfigSec.bField = m_BField;
  VertexFinderSec finder(std::move(vertexfinderConfigSec));
  // Instantiate the finder
  auto stateSec = finder.makeState(m_fieldctx);

  VertexFinderOptionsSec vfOptions(m_geoctx, m_fieldctx);

  // Construct ConstTrackContainer from underlying containers
  auto trackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(*trackStates);
  auto trackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>(*tracks);
  ActsExamples::ConstTrackContainer constTracks(trackContainer, trackStateContainer);

  std::vector<Acts::InputTrack> inputTracks;
  std::vector<Acts::BoundTrackParameters> trackParameters;
  trackParameters.reserve(constTracks.size());

  for (const auto& track : constTracks) {
    // Create BoundTrackParameters and store it
    trackParameters.emplace_back(track.referenceSurface().getSharedPtr(), track.parameters(),
                                 track.covariance(), track.particleHypothesis());
    // Create InputTrack from stored parameters
    inputTracks.emplace_back(&trackParameters.back());
    trace("Track local position at input = {} mm, {} mm",
          track.parameters()[Acts::eBoundLoc0] / Acts::UnitConstants::mm,
          track.parameters()[Acts::eBoundLoc1] / Acts::UnitConstants::mm);
  }

  std::vector<Acts::Vertex> vertices;
  auto result = finder.find(inputTracks, vfOptions, stateSec);
  if (result.ok()) {
    vertices = std::move(result.value());
  }

  for (const auto& vtx : vertices) {
    edm4eic::Cov4f cov(vtx.fullCovariance()(0, 0), vtx.fullCovariance()(1, 1),
                       vtx.fullCovariance()(2, 2), vtx.fullCovariance()(3, 3),
                       vtx.fullCovariance()(0, 1), vtx.fullCovariance()(0, 2),
                       vtx.fullCovariance()(0, 3), vtx.fullCovariance()(1, 2),
                       vtx.fullCovariance()(1, 3), vtx.fullCovariance()(2, 3));
    auto eicvertex = prmVertices.create();
    eicvertex.setType(1); // boolean flag if vertex is primary vertex of event
    eicvertex.setChi2(static_cast<float>(vtx.fitQuality().first)); // chi2
    eicvertex.setNdf(static_cast<float>(vtx.fitQuality().second)); // ndf
    eicvertex.setPosition({
        static_cast<float>(vtx.position().x()),
        static_cast<float>(vtx.position().y()),
        static_cast<float>(vtx.position().z()),
        static_cast<float>(vtx.time()),
    });                              // vtxposition
    eicvertex.setPositionError(cov); // covariance

    for (const auto& t : vtx.tracks()) {
      const auto& par = vertexfinderConfigSec.extractParameters(t.originalParams);
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
          for (const auto& par : trkPars) {
            double EPSILON = std::numeric_limits<double>::epsilon(); // mm
            if (std::abs((par.getLoc().a / edm4eic::unit::mm) - (loc_a / Acts::UnitConstants::mm)) <
                    EPSILON &&
                std::abs((par.getLoc().b / edm4eic::unit::mm) - (loc_b / Acts::UnitConstants::mm)) <
                    EPSILON) {
              trace("From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
                    par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
              eicvertex.addToAssociatedParticles(part);
            } // endif
          } // end for par
        } // end for trk
      } // end for part
    } // end for t
    debug("One AMVF vertex found at (x,y,z) = ({}, {}, {}) mm.",
          vtx.position().x() / Acts::UnitConstants::mm,
          vtx.position().y() / Acts::UnitConstants::mm,
          vtx.position().z() / Acts::UnitConstants::mm);
  } // end for vtx
}

void SecondaryVertexFinder::calculateSecondaryVertex(
    const edm4eic::ReconstructedParticleCollection& reconParticles,
    const Acts::ConstVectorMultiTrajectory* trackStates,
    const Acts::ConstVectorTrackContainer* tracks, Acts::EigenStepper<> stepperSec,
    edm4eic::VertexCollection& secVertices) const {
  // Convert algorithm log level to Acts log level
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("AMVF", acts_level));

  // Geometry and field contexts
  const auto& m_geoctx   = m_geoSvc->getActsGeometryContext();
  const auto& m_fieldctx = m_geoSvc->getActsMagneticFieldContext();

  // Set-up the propagator
  using PropagatorSec = Acts::Propagator<Acts::EigenStepper<>>;

  // Set up propagator with void navigator
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::VoidNavigator{},
                                                       logger().cloneWithSuffix("Prop"));

  // Set up track density used during vertex seeding
  Acts::AdaptiveGridTrackDensity::Config trkDensityConfig;
  // Bin extent in z-direction
  trkDensityConfig.spatialBinExtent = m_cfg.spatialBinExtent;
  // Bin extent in t-direction
  trkDensityConfig.temporalBinExtent = m_cfg.temporalBinExtent;
  trkDensityConfig.useTime           = m_cfg.useTime;
  Acts::AdaptiveGridTrackDensity trkDensity(trkDensityConfig);

  // Setup the track linearizer
  LinearizerSec::Config linearizerConfigSec(m_BField, propagatorSec);
  std::unique_ptr<const Acts::Logger> linearizer_log = logger().cloneWithSuffix("Linearizer");
  LinearizerSec linearizerSec(linearizerConfigSec, std::move(linearizer_log));

  //Staring multivertex fitter
  // Set up deterministic annealing with user-defined temperatures
  Acts::AnnealingUtility::Config annealingConfig;
  annealingConfig.setOfTemperatures = {9., 1.0};
  Acts::AnnealingUtility annealingUtility(annealingConfig);
  // Setup the vertex fitter
  ImpactPointEstimator::Config ipEstConfig(m_BField, propagatorSec);
  ImpactPointEstimator ipEst(ipEstConfig);
  VertexFitterSec::Config vertexFitterConfigSec(ipEst);

  vertexFitterConfigSec.annealingTool     = annealingUtility;
  vertexFitterConfigSec.minWeight         = m_cfg.minWeight;
  vertexFitterConfigSec.maxDistToLinPoint = m_cfg.maxDistToLinPoint;
  vertexFitterConfigSec.doSmoothing       = m_cfg.doSmoothing;
  vertexFitterConfigSec.useTime           = m_cfg.useTime;
  vertexFitterConfigSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterConfigSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);
  VertexFitterSec vertexFitterSec(std::move(vertexFitterConfigSec));

  // Set up vertex seeder and finder
  using seedFinder = Acts::AdaptiveGridDensityVertexFinder;
  std::shared_ptr<const Acts::IVertexFinder> seeder;
  seedFinder::Config seederConfig(trkDensity);
  seederConfig.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  seeder = std::make_shared<seedFinder>(seedFinder::Config{seederConfig});

  VertexFinderSec::Config vertexfinderConfigSec(std::move(vertexFitterSec), std::move(seeder),
                                                std::move(ipEst), m_BField);

  // The vertex finder state
  // Set the initial variance of the 4D vertex position. Since time is on a
  // numerical scale, we have to provide a greater value in the corresponding
  // dimension.
  vertexfinderConfigSec.initialVariances = m_cfg.initialVariances;
  //Use time for Sec. Vertex
  vertexfinderConfigSec.useTime                    = m_cfg.useTime;
  vertexfinderConfigSec.useSeedConstraint          = m_cfg.useSeedConstraint;
  vertexfinderConfigSec.tracksMaxZinterval         = m_cfg.tracksMaxZintervalSec;
  vertexfinderConfigSec.maxIterations              = m_cfg.maxSecIterations;
  vertexfinderConfigSec.doFullSplitting            = m_cfg.doFullSplitting;
  vertexfinderConfigSec.tracksMaxSignificance      = m_cfg.tracksMaxSignificance;
  vertexfinderConfigSec.maxMergeVertexSignificance = m_cfg.maxMergeVertexSignificance;

  if (m_cfg.useTime) {
    // When using time, we have an extra contribution to the chi2 by the time
    // coordinate.
    vertexfinderConfigSec.tracksMaxSignificance      = m_cfg.tracksMaxSignificance;
    vertexfinderConfigSec.maxMergeVertexSignificance = m_cfg.maxMergeVertexSignificance;
  }

  vertexfinderConfigSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();

  vertexfinderConfigSec.bField = m_BField;
  VertexFinderSec finder(std::move(vertexfinderConfigSec));
  // Instantiate the finder
  auto stateSec = finder.makeState(m_fieldctx);

  VertexFinderOptionsSec vfOptions(m_geoctx, m_fieldctx);

  //--->Add Prm Vertex container here
  // Construct ConstTrackContainer from underlying containers
  auto trackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(*trackStates);
  auto trackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>(*tracks);
  ActsExamples::ConstTrackContainer constTracks(trackContainer, trackStateContainer);

  // Build BoundTrackParameters for all tracks upfront
  std::vector<Acts::BoundTrackParameters> allTrackParameters;
  allTrackParameters.reserve(constTracks.size());
  for (const auto& track : constTracks) {
    allTrackParameters.emplace_back(track.referenceSurface().getSharedPtr(), track.parameters(),
                                    track.covariance(), track.particleHypothesis());
  }

  std::vector<Acts::InputTrack> inputTracks;
  for (unsigned int i = 0; i < allTrackParameters.size() - 1; i++) {
    for (unsigned int j = i + 1; j < allTrackParameters.size(); j++) {
      inputTracks.emplace_back(&allTrackParameters[i]);
      inputTracks.emplace_back(&allTrackParameters[j]);
      // run the vertex finder for both tracks
      std::vector<Acts::Vertex> verticesSec;
      auto resultSecondary = finder.find(inputTracks, vfOptions, stateSec);
      if (resultSecondary.ok()) {
        verticesSec = std::move(resultSecondary.value());
      }

      for (const auto& secvertex : verticesSec) {
        edm4eic::Cov4f cov(secvertex.fullCovariance()(0, 0), secvertex.fullCovariance()(1, 1),
                           secvertex.fullCovariance()(2, 2), secvertex.fullCovariance()(3, 3),
                           secvertex.fullCovariance()(0, 1), secvertex.fullCovariance()(0, 2),
                           secvertex.fullCovariance()(0, 3), secvertex.fullCovariance()(1, 2),
                           secvertex.fullCovariance()(1, 3), secvertex.fullCovariance()(2, 3));
        auto eicvertex = secVertices.create();
        eicvertex.setType(0); // boolean flag if vertex is primary vertex of event
        eicvertex.setChi2(static_cast<float>(secvertex.fitQuality().first)); // chi2
        eicvertex.setNdf(static_cast<float>(secvertex.fitQuality().second)); // ndf
        eicvertex.setPosition({
            static_cast<float>(secvertex.position().x()),
            static_cast<float>(secvertex.position().y()),
            static_cast<float>(secvertex.position().z()),
            static_cast<float>(secvertex.time()),
        });                              // vtxposition
        eicvertex.setPositionError(cov); // covariance

        for (const auto& t : secvertex.tracks()) {
          const auto& par = vertexfinderConfigSec.extractParameters(t.originalParams);
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
              for (const auto& par : trkPars) {
                double EPSILON = std::numeric_limits<double>::epsilon(); // mm
                if (std::abs((par.getLoc().a / edm4eic::unit::mm) -
                             (loc_a / Acts::UnitConstants::mm)) < EPSILON &&
                    std::abs((par.getLoc().b / edm4eic::unit::mm) -
                             (loc_b / Acts::UnitConstants::mm)) < EPSILON) {
                  trace("From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
                        par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
                  eicvertex.addToAssociatedParticles(part);
                } // endif
              } // end for par
            } // end for trk
          } // end for part
        } // end for t
      } // end for secvertex

      // empty the vector for the next set of tracks
      inputTracks.clear();
    } //end of int j=i+1
  } // end of int i=0; i<constTracks->size()
}

} // namespace eicrecon
