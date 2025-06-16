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
#if Acts_VERSION_MAJOR >= 32
#include <Acts/Propagator/VoidNavigator.hpp>
#else
#include <Acts/Propagator/detail/VoidPropagatorComponents.hpp>
#endif
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

std::tuple<std::unique_ptr<edm4eic::VertexCollection>,
           //std::unique_ptr<edm4eic::TrackCollection>,
           std::unique_ptr<edm4eic::VertexCollection>>
eicrecon::SecondaryVertexFinder::produce(
    const edm4eic::ReconstructedParticleCollection* recotracks,
    std::vector<const ActsExamples::Trajectories*> trajectories) {

  auto primaryVertices = std::make_unique<edm4eic::VertexCollection>();
  auto outputVertices  = std::make_unique<edm4eic::VertexCollection>();

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("SVF", m_log));

  Acts::EigenStepper<> stepperSec(m_BField);

  // Set-up the propagator
  using PropagatorSec = Acts::Propagator<Acts::EigenStepper<>>;
  //using PropagatorOptionsSec = Acts::PropagatorOptions<>;

  // Set up propagator with void navigator
#if Acts_VERSION_MAJOR >= 32
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::VoidNavigator{},
                                                       logger().cloneWithSuffix("PropSec"));
#else
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::detail::VoidNavigator{},
                                                       logger().cloneWithSuffix("PropSec"));
#endif

  //set up Impact estimator
#if Acts_VERSION_MAJOR >= 33
  using ImpactPointEstimator = Acts::ImpactPointEstimator;
  using LinearizerSec        = Acts::HelicalTrackLinearizer;
  using VertexFitterSec      = Acts::AdaptiveMultiVertexFitter;
  //using VertexSeederSec      = Acts::TrackDensityVertexFinder;
  // The vertex finder type
  using VertexFinderSec        = Acts::AdaptiveMultiVertexFinder;
  using VertexFinderOptionsSec = Acts::VertexingOptions;
#else
  using ImpactPointEstimator =
      Acts::ImpactPointEstimator<Acts::BoundTrackParameters, PropagatorSec>;
  using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  using VertexFitterSec =
      Acts::AdaptiveMultiVertexFitter<Acts::BoundTrackParameters, LinearizerSec>;
  using VertexSeederSec = Acts::Acts::ZScanVertexFinder<VertexFitterSec>;
  // The vertex finder type
  using VertexFinderSec        = Acts::AdaptiveMultiVertexFinder<VertexFitterSec, VertexSeederSec>;
  using VertexFinderOptionsSec = Acts::VertexingOptions<Acts::BoundTrackParameters>;
#endif

  // Set up track density used during vertex seeding
  Acts::AdaptiveGridTrackDensity::Config trkDensityCfg;
  // Bin extent in z-direction
  trkDensityCfg.spatialBinExtent = 25 * Acts::UnitConstants::um;
  // Bin extent in t-direction
  //trkDensityCfg.temporalBinExtent = 19. * Acts::UnitConstants::mm;
  trkDensityCfg.useTime = false; //m_cfg.useTime;
  Acts::AdaptiveGridTrackDensity trkDensity(trkDensityCfg);

  // using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  // Setup the track linearizer
  LinearizerSec::Config linearizerCfgSec(m_BField, propagatorSec);
  LinearizerSec linearizerSec(linearizerCfgSec, logger().cloneWithSuffix("HelLinSec"));

  //Trying multivertex fitter here...
  // Set up deterministic annealing with user-defined temperatures
  Acts::AnnealingUtility::Config annealingCfg;
  annealingCfg.setOfTemperatures = {9., 1.0};
  Acts::AnnealingUtility annealingUtility(annealingCfg);
  // Setup the vertex fitter
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagatorSec);
  ImpactPointEstimator ipEst(ipEstCfg);
  VertexFitterSec::Config vertexFitterCfgSec(ipEst);

  vertexFitterCfgSec.annealingTool     = annealingUtility;
  vertexFitterCfgSec.minWeight         = 1e-04;
  vertexFitterCfgSec.maxDistToLinPoint = 5.5 * Acts::UnitConstants::mm;
  vertexFitterCfgSec.doSmoothing       = true;
  vertexFitterCfgSec.useTime           = false; //m_cfg.useTime;
#if Acts_VERSION_MAJOR >= 33
  vertexFitterCfgSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfgSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);
#endif
  VertexFitterSec vertexFitterSec(std::move(vertexFitterCfgSec));

#if Acts_VERSION_MAJOR >= 33

  // Set up vertex seeder and finder
  using seedFinder = Acts::AdaptiveGridDensityVertexFinder;
  std::shared_ptr<const Acts::IVertexFinder> seeder;
  seedFinder::Config seederCfg(trkDensity);
  seederCfg.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  seeder = std::make_shared<seedFinder>(seedFinder::Config{seederCfg});
#else
  // Set up the vertex seed finder
  VertexSeederSec::Config seedercfg(ipEst);
  VertexSeederSec seeder(seederCfg);
#endif

  VertexFinderSec::Config vertexfinderCfgSec(std::move(vertexFitterSec), std::move(seeder),
                                             std::move(ipEst), m_BField);

  // The vertex finder state
#if Acts_VERSION_MAJOR >= 31
#if Acts_VERSION_MAJOR >= 33
  // Set the initial variance of the 4D vertex position. Since time is on a
  // numerical scale, we have to provide a greater value in the corresponding
  // dimension.
  vertexfinderCfgSec.initialVariances << 1e+2, 1e+2, 1e+2, 1e+8;
  //Use time for Sec. Vertex
  vertexfinderCfgSec.useTime            = false;
  vertexfinderCfgSec.tracksMaxZinterval = 35 * Acts::UnitConstants::mm;
  vertexfinderCfgSec.maxIterations      = 1000;
  vertexfinderCfgSec.doFullSplitting    = false;
  // 5 corresponds to a p-value of ~0.92 using `chi2(x=5,ndf=2)`
  vertexfinderCfgSec.tracksMaxSignificance      = 6.7;
  vertexfinderCfgSec.maxMergeVertexSignificance = 5;

  if (m_cfg.useTime) {
    // When using time, we have an extra contribution to the chi2 by the time
    // coordinate. We thus need to increase tracksMaxSignificance (i.e., the
    // maximum chi2 that a track can have to be associated with a vertex).
    vertexfinderCfgSec.tracksMaxSignificance = 6.7;
    // Using the same p-value for 2 dof instead of 1.
    // 5 corresponds to a p-value of ~0.92 using `chi2(x=5,ndf=2)`
    vertexfinderCfgSec.maxMergeVertexSignificance = 5;
  }

  vertexfinderCfgSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfgSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);

#if Acts_VERSION_MAJOR >= 36
  vertexfinderCfgSec.bField = m_BField;
#else
  vertexfinderCfgSec.bField = std::dynamic_pointer_cast<Acts::MagneticFieldProvider>(
      std::const_pointer_cast<eicrecon::BField::DD4hepBField>(m_BField));
#endif
#endif
  VertexFinderSec vertexfinderSec(std::move(vertexfinderCfgSec)); //,logger().clone());
#else
  VertexFinder finder(std::move(vertexfinderCfgSec));
  typename VertexFinderSec::State stateSec(*m_BField, m_fieldctx);
#endif
  // Instantiate the finder
#if Acts_VERSION_MAJOR >= 33
  auto stateSec = vertexfinderSec.makeState(m_fieldctx);
#else
  typename Acts::IVertexFinder::State stateSec;
#endif

  VertexFinderOptionsSec vfOptions(m_geoctx, m_fieldctx);

#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::InputTrack> inputTracks;
#else
  std::vector<const Acts::BoundTrackParameters*> inputTrackPointersSecondary;
#endif
  // Calculate primary vertex using AMVF
  primaryVertices = calcPrimaryVtx(recotracks, trajectories, vertexfinderSec, vfOptions,
                                   vertexfinderCfgSec, stateSec);
  // Primary vertex collection container to be used in Sec. Vertex fitting
  //std::vector<Acts::Vertex> primvtx = getprmvtx();
  //Evaluate Two Track Vertex fitting
  outputVertices = calcSecVtx(recotracks, trajectories, vertexfinderSec, vfOptions,
                              vertexfinderCfgSec, stateSec); // Will fix this later, primvtx);

  //return std::move(outputVertices);
  return std::make_tuple(std::move(primaryVertices), std::move(outputVertices));
}
