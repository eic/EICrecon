// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Dongwi H. Dongwi (Bishoy)

#include "SecondaryVertexFinder.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Utilities/AnnealingUtility.hpp>
#include <Acts/Utilities/detail/ContextType.hpp>
#include <Acts/Vertexing/LinearizedTrack.hpp>
#include <Acts/Propagator/VoidNavigator.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <Eigen/Core>
#include <tuple>
#include <utility>

#include "Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp"
#include "Acts/Vertexing/AdaptiveGridTrackDensity.hpp"
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"
#include "extensions/spdlog/SpdlogToActs.h"

void eicrecon::SecondaryVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log    = log;
  m_geoSvc = geo_svc;
  m_BField =
      std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
  m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);
}

std::tuple<std::unique_ptr<edm4eic::VertexCollection>, std::unique_ptr<edm4eic::VertexCollection>>
eicrecon::SecondaryVertexFinder::produce(
    const edm4eic::ReconstructedParticleCollection* recotracks,
    std::vector<const ActsExamples::Trajectories*> trajectories) {

  auto primaryVertices = std::make_unique<edm4eic::VertexCollection>();
  auto outputVertices  = std::make_unique<edm4eic::VertexCollection>();

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("SVF", m_log));

  Acts::EigenStepper<> stepperSec(m_BField);

  // Set-up the propagator
  using PropagatorSec = Acts::Propagator<Acts::EigenStepper<>>;

  // Set up propagator with void navigator
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::VoidNavigator{},
                                                       logger().cloneWithSuffix("PropSec"));

  //set up Impact estimator
  using ImpactPointEstimator   = Acts::ImpactPointEstimator;
  using LinearizerSec          = Acts::HelicalTrackLinearizer;
  using VertexFitterSec        = Acts::AdaptiveMultiVertexFitter;
  using VertexFinderSec        = Acts::AdaptiveMultiVertexFinder;
  using VertexFinderOptionsSec = Acts::VertexingOptions;

  // Set up track density used during vertex seeding
  Acts::AdaptiveGridTrackDensity::Config trkDensityConfig;
  // Bin extent in z-direction
  trkDensityConfig.spatialBinExtent = 25 * Acts::UnitConstants::um;
  // Bin extent in t-direction
  trkDensityConfig.temporalBinExtent = 19. * Acts::UnitConstants::mm;
  trkDensityConfig.useTime           = false;
  Acts::AdaptiveGridTrackDensity trkDensity(trkDensityConfig);

  // Setup the track linearizer
  LinearizerSec::Config linearizerConfigSec(m_BField, propagatorSec);
  LinearizerSec linearizerSec(linearizerConfigSec, logger().cloneWithSuffix("HelLinSec"));

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
  vertexFitterConfigSec.minWeight         = 1e-04;
  vertexFitterConfigSec.maxDistToLinPoint = 5.5 * Acts::UnitConstants::mm;
  vertexFitterConfigSec.doSmoothing       = true;
  vertexFitterConfigSec.useTime           = false;
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
  vertexfinderConfigSec.initialVariances = {1e+2, 1e+2, 1e+2, 1e+8};
  //Use time for Sec. Vertex
  vertexfinderConfigSec.useTime            = false;
  vertexfinderConfigSec.tracksMaxZinterval = 35 * Acts::UnitConstants::mm;
  vertexfinderConfigSec.maxIterations      = 500;
  vertexfinderConfigSec.doFullSplitting    = false;
  // 5 corresponds to a p-value of ~0.92 using `chi2(x=5,ndf=2)`
  vertexfinderConfigSec.tracksMaxSignificance      = 6.7;
  vertexfinderConfigSec.maxMergeVertexSignificance = 5;

  if (m_cfg.useTime) {
    // When using time, we have an extra contribution to the chi2 by the time
    // coordinate.
    vertexfinderConfigSec.tracksMaxSignificance      = 6.7;
    vertexfinderConfigSec.maxMergeVertexSignificance = 5;
  }

  vertexfinderConfigSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterConfigSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);

#if Acts_VERSION_MAJOR >= 36
  vertexfinderConfigSec.bField = m_BField;
#else
  vertexfinderConfigSec.bField = std::dynamic_pointer_cast<Acts::MagneticFieldProvider>(
      std::const_pointer_cast<eicrecon::BField::DD4hepBField>(m_BField));
#endif
  VertexFinderSec vertexfinderSec(std::move(vertexfinderConfigSec));
  // Instantiate the finder
  auto stateSec = vertexfinderSec.makeState(m_fieldctx);

  VertexFinderOptionsSec vfOptions(m_geoctx, m_fieldctx);

  std::vector<Acts::InputTrack> inputTracks;
  // Calculate primary vertex using AMVF
  primaryVertices = calculatePrimaryVertex(recotracks, trajectories, vertexfinderSec, vfOptions,
                                           vertexfinderConfigSec, stateSec);
  // Primary vertex collection container to be used in Sec. Vertex fitting
  outputVertices = calculateSecondaryVertex(recotracks, trajectories, vertexfinderSec, vfOptions,
                                            vertexfinderConfigSec, stateSec);

  return std::make_tuple(std::move(primaryVertices), std::move(outputVertices));
}
