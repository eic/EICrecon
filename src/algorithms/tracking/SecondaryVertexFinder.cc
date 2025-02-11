// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "SecondaryVertexFinder.h"
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <JANA/Utils/JTypeInfo.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/GenericParticleHypothesis.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#if Acts_VERSION_MAJOR >= 32
#include <Acts/Propagator/VoidNavigator.hpp>
#else
#include <Acts/Propagator/detail/VoidPropagatorComponents.hpp>
#endif
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Utilities/VectorHelpers.hpp>
#include <Acts/Vertexing/FullBilloirVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include "Acts/Vertexing/TrackDensityVertexFinder.hpp"
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/IterativeVertexFinder.hpp>
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/ZScanVertexFinder.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <algorithm>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov4f.h>
#include <math.h>
#include <optional>
#include <tuple>
#include <utility>

#include "edm4eic/ReconstructedParticle.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"
#include "services/rootfile/RootFile_service.h"
#include "extensions/spdlog/SpdlogToActs.h"

void eicrecon::SecondaryVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log = log;

  m_geoSvc = geo_svc;

  m_BField =
      std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
  m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);

}

std::tuple<
  std::unique_ptr<edm4eic::VertexCollection>,
  //std::unique_ptr<edm4eic::TrackCollection>,
  std::unique_ptr<edm4eic::VertexCollection>
> eicrecon::SecondaryVertexFinder::produce(
    const edm4eic::ReconstructedParticleCollection* recotracks,
    std::vector<const ActsExamples::Trajectories*> trajectories) {
/*
std::unique_ptr<edm4eic::VertexCollection> eicrecon::SecondaryVertexFinder::produce(
    const edm4eic::ReconstructedParticleCollection* recotracks,
    std::vector<const ActsExamples::Trajectories*> trajectories) {
*/

  //auto primaryVertices = std::make_unique<edm4eic::TrackCollection>();
  auto primaryVertices = std::make_unique<edm4eic::VertexCollection>();
  auto outputVertices  = std::make_unique<edm4eic::VertexCollection>();
  //std::vector<edm4eic::VertexCollection*> outVertices;

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("SVF", m_log));

  Acts::EigenStepper<> stepperSec(m_BField);

  // Set-up the propagator
  using PropagatorSec        = Acts::Propagator<Acts::EigenStepper<>>;
  using PropagatorOptionsSec = Acts::PropagatorOptions<>;

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
  using LinearizerSec = Acts::HelicalTrackLinearizer;
  using VertexFitterSec = Acts::AdaptiveMultiVertexFitter;
  using VertexSeederSec = Acts::TrackDensityVertexFinder;
  // The vertex finder type
  using VertexFinderSec = Acts::AdaptiveMultiVertexFinder;
  using VertexFinderOptionsSec = Acts::VertexingOptions;
#else
  using ImpactPointEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, PropagatorSec>;
  using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  using VertexFitterSec = Acts::AdaptiveMultiVertexFitter<Acts::BoundTrackParameters, LinearizerSec>;
  using VertexSeederSec = Acts::Acts::ZScanVertexFinder<VertexFitterSec>;
  // The vertex finder type
  using VertexFinderSec = Acts::AdaptiveMultiVertexFinder<VertexFitterSec, VertexSeederSec>;
  using VertexFinderOptionsSec = Acts::VertexingOptions<Acts::BoundTrackParameters>;
#endif

  // using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  // Setup the track linearizer
  LinearizerSec::Config linearizerCfgSec(m_BField, propagatorSec);
  LinearizerSec linearizerSec(linearizerCfgSec, logger().cloneWithSuffix("HelLinSec"));

  //Trying multivertex fitter here...
  // Setup the vertex fitter
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagatorSec);
  ImpactPointEstimator ipEst(ipEstCfg);
  VertexFitterSec::Config vertexFitterCfgSec(ipEst);
#if Acts_VERSION_MAJOR >= 33
  vertexFitterCfgSec.extractParameters
    .connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfgSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(
    &linearizerSec);
#endif
  VertexFitterSec vertexFitterSec(vertexFitterCfgSec);

#if Acts_VERSION_MAJOR >= 33
  // Set up the vertex seed finder
  //VertexSeederSec::Config seedercfg(ipEst);
  std::shared_ptr<const Acts::IVertexFinder> seeder;
  Acts::GaussianTrackDensity::Config seederCfg;
  seederCfg.extractParameters
    .connect<&Acts::InputTrack::extractParameters>();
  seeder = std::make_shared<VertexSeederSec>(VertexSeederSec::Config{seederCfg});
#else
  // Set up the vertex seed finder
  VertexSeederSec::Config seedercfg(ipEst);
  VertexSeederSec seeder(seederCfg);
#endif


  VertexFinderSec::Config vertexfinderCfgSec(std::move(vertexFitterSec),
                              std::move(seeder), 
                              std::move(ipEst),
                              m_BField);

  // The vertex finder state
#if Acts_VERSION_MAJOR >= 31
  #if Acts_VERSION_MAJOR >= 33
  // Set the initial variance of the 4D vertex position. Since time is on a
  // numerical scale, we have to provide a greater value in the corresponding
  // dimension.
  vertexfinderCfgSec.initialVariances<<1e+2, 1e+2, 1e+2, 1e+8;
  // We do not want to use a beamspot constraint here
  //vertexfinderCfgSec.useBeamSpotConstraint = false;
  vertexfinderCfgSec.tracksMaxZinterval=35. * Acts::UnitConstants::mm;
  vertexfinderCfgSec.maxIterations=200;
  vertexfinderCfgSec.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfgSec.trackLinearizer.connect<&LinearizerSec::linearizeTrack>(&linearizerSec);
  #if Acts_VERSION_MAJOR >= 36
  vertexfinderCfgSec.field = m_BField;
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
  primaryVertices=
    calcPrimaryVtx(recotracks,trajectories,vertexfinderSec,vfOptions,vertexfinderCfgSec,stateSec);
  // Primary vertex collection container to be used in Sec. Vertex fitting
  std::vector<Acts::Vertex> primvtx=getprmvtx();
  //Evaluate Two Track Vertex fitting
  outputVertices=
    calcSecVtx(recotracks,trajectories,vertexfinderSec,vfOptions,vertexfinderCfgSec,stateSec,primvtx);

  //return std::move(outputVertices);
  return std::make_tuple(std::move(primaryVertices),std::move(outputVertices));
}
