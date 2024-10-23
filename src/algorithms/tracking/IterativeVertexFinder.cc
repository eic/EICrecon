// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "IterativeVertexFinder.h"

#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/GenericParticleHypothesis.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <boost/move/utility_core.hpp>
#include <edm4eic/Track.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
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
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/IterativeVertexFinder.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/ZScanVertexFinder.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov4f.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <algorithm>
#include <cmath>
#include <optional>
#include <tuple>
#include <utility>

#include "extensions/spdlog/SpdlogToActs.h"

void eicrecon::IterativeVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log = log;

  m_geoSvc = geo_svc;

  m_BField =
      std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
  m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);
}

std::unique_ptr<edm4eic::VertexCollection> eicrecon::IterativeVertexFinder::produce(
    std::vector<const ActsExamples::Trajectories*> trajectories,
    const edm4eic::ReconstructedParticleCollection* reconParticles) {

  auto outputVertices = std::make_unique<edm4eic::VertexCollection>();

  using Propagator        = Acts::Propagator<Acts::EigenStepper<>>;
  using PropagatorOptions = Acts::PropagatorOptions<>;
#if Acts_VERSION_MAJOR >= 33
  using Linearizer        = Acts::HelicalTrackLinearizer;
  using VertexFitter      = Acts::FullBilloirVertexFitter;
  using ImpactPointEstimator = Acts::ImpactPointEstimator;
  using VertexSeeder         = Acts::ZScanVertexFinder;
  using VertexFinder         = Acts::IterativeVertexFinder;
  using VertexFinderOptions  = Acts::VertexingOptions;
#else
  using Linearizer        = Acts::HelicalTrackLinearizer<Propagator>;
  using VertexFitter      = Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, Linearizer>;
  using ImpactPointEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, Propagator>;
  using VertexSeeder         = Acts::ZScanVertexFinder<VertexFitter>;
  using VertexFinder         = Acts::IterativeVertexFinder<VertexFitter, VertexSeeder>;
  using VertexFinderOptions  = Acts::VertexingOptions<Acts::BoundTrackParameters>;
#endif

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("IVF", m_log));

  Acts::EigenStepper<> stepper(m_BField);

  // Set up propagator with void navigator
#if Acts_VERSION_MAJOR >= 32
  auto propagator = std::make_shared<Propagator>(
    stepper, Acts::VoidNavigator{}, logger().cloneWithSuffix("Prop"));
#else
  auto propagator = std::make_shared<Propagator>(
    stepper, Acts::detail::VoidNavigator{}, logger().cloneWithSuffix("Prop"));
#endif
  Acts::PropagatorOptions opts(m_geoctx, m_fieldctx);

  // Setup the track linearizer
#if Acts_VERSION_MAJOR >= 33
  Linearizer::Config linearizerCfg;
  linearizerCfg.bField = m_BField;
  linearizerCfg.propagator = propagator;
#else
  Linearizer::Config linearizerCfg(m_BField, propagator);
#endif
  Linearizer linearizer(linearizerCfg, logger().cloneWithSuffix("HelLin"));

  // Setup the vertex fitter
  VertexFitter::Config vertexFitterCfg;
#if Acts_VERSION_MAJOR >= 33
  vertexFitterCfg.extractParameters
    .connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfg.trackLinearizer.connect<&Linearizer::linearizeTrack>(
      &linearizer);
#endif
  VertexFitter vertexFitter(vertexFitterCfg);

  // Setup the seed finder
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagator);
  ImpactPointEstimator ipEst(ipEstCfg);
  VertexSeeder::Config seederCfg(ipEst);
#if Acts_VERSION_MAJOR >= 33
  seederCfg.extractParameters
    .connect<&Acts::InputTrack::extractParameters>();
  auto seeder = std::make_shared<VertexSeeder>(seederCfg);
#else
  VertexSeeder seeder(seederCfg);
#endif

  // Set up the actual vertex finder
  VertexFinder::Config finderCfg(
    std::move(vertexFitter),
#if Acts_VERSION_MAJOR < 33
    std::move(linearizer),
#endif
    std::move(seeder),
    std::move(ipEst));
  finderCfg.maxVertices                 = m_cfg.maxVertices;
  finderCfg.reassignTracksAfterFirstFit = m_cfg.reassignTracksAfterFirstFit;
#if Acts_VERSION_MAJOR >= 31
 #if Acts_VERSION_MAJOR >= 33
  finderCfg.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  finderCfg.trackLinearizer.connect<&Linearizer::linearizeTrack>(&linearizer);
  #if Acts_VERSION_MAJOR >= 36
  finderCfg.field = m_BField;
  #else
  finderCfg.field = std::dynamic_pointer_cast<Acts::MagneticFieldProvider>(
    std::const_pointer_cast<eicrecon::BField::DD4hepBField>(m_BField));
  #endif
 #endif
  VertexFinder finder(std::move(finderCfg));
#else
  VertexFinder finder(finderCfg);
#endif
#if Acts_VERSION_MAJOR >= 33
  Acts::IVertexFinder::State state(
    std::in_place_type<VertexFinder::State>,
    *m_BField,
    m_fieldctx);
#else
  VertexFinder::State state(*m_BField, m_fieldctx);
#endif
  VertexFinderOptions finderOpts(m_geoctx, m_fieldctx);

#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::InputTrack> inputTracks;
#else
  std::vector<const Acts::BoundTrackParameters*> inputTrackPointers;
#endif

  for (const auto& trajectory : trajectories) {
    auto tips = trajectory->tips();
    if (tips.empty()) {
      continue;
    }
    /// CKF can provide multiple track trajectories for a single input seed
    for (auto& tip : tips) {
      ActsExamples::TrackParameters par = trajectory->trackParameters(tip);

#if Acts_VERSION_MAJOR >= 33
      inputTracks.emplace_back(&(trajectory->trackParameters(tip)));
#else
      inputTrackPointers.push_back(&(trajectory->trackParameters(tip)));
#endif
      m_log->trace("Track local position at input = {} mm, {} mm", par.localPosition().x() / Acts::UnitConstants::mm, par.localPosition().y() / Acts::UnitConstants::mm);
    }
  }

#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::Vertex> vertices;
  auto result = finder.find(inputTracks, finderOpts, state);
#else
  std::vector<Acts::Vertex<Acts::BoundTrackParameters>> vertices;
  auto result = finder.find(inputTrackPointers, finderOpts, state);
#endif
  if (result.ok()) {
    vertices = std::move(result.value());
  }

  for (const auto& vtx : vertices) {
    edm4eic::Cov4f cov(vtx.fullCovariance()(0,0), vtx.fullCovariance()(1,1), vtx.fullCovariance()(2,2), vtx.fullCovariance()(3,3),
                       vtx.fullCovariance()(0,1), vtx.fullCovariance()(0,2), vtx.fullCovariance()(0,3),
                       vtx.fullCovariance()(1,2), vtx.fullCovariance()(1,3),
                       vtx.fullCovariance()(2,3));
    auto eicvertex = outputVertices->create();
    eicvertex.setType(1);                                  // boolean flag if vertex is primary vertex of event
    eicvertex.setChi2((float)vtx.fitQuality().first);      // chi2
    eicvertex.setNdf((float)vtx.fitQuality().second);      // ndf
    eicvertex.setPosition({
         (float)vtx.position().x(),
         (float)vtx.position().y(),
         (float)vtx.position().z(),
         (float)vtx.time(),
    }); // vtxposition
    eicvertex.setPositionError(cov);                          // covariance

    for (const auto& t : vtx.tracks()) {
#if Acts_VERSION_MAJOR >= 33
      const auto& par = finderCfg.extractParameters(t.originalParams);
#else
      const auto& par = *t.originalParams;
#endif
      m_log->trace("Track local position from vertex = {} mm, {} mm", par.localPosition().x() / Acts::UnitConstants::mm, par.localPosition().y() / Acts::UnitConstants::mm);
      float loc_a = par.localPosition().x();
      float loc_b = par.localPosition().y();

      for (const auto& part : *reconParticles) {
        const auto& tracks = part.getTracks();
        for (const auto trk : tracks) {
          const auto& traj = trk.getTrajectory();
          const auto& trkPars = traj.getTrackParameters();
          for (const auto par : trkPars) {
            const double EPSILON = 1.0e-4; // mm
            if (fabs((par.getLoc().a / edm4eic::unit::mm) - (loc_a / Acts::UnitConstants::mm)) < EPSILON
              && fabs((par.getLoc().b / edm4eic::unit::mm) - (loc_b / Acts::UnitConstants::mm)) < EPSILON) {
              m_log->trace("From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm", par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
              eicvertex.addToAssociatedParticles(part);
            } // endif
          } // end for par
        } // end for trk
      } // end for part
    } // end for t
    m_log->debug("One vertex found at (x,y,z) = ({}, {}, {}) mm.", vtx.position().x() / Acts::UnitConstants::mm, vtx.position().y() / Acts::UnitConstants::mm, vtx.position().z() / Acts::UnitConstants::mm);

  } // end for vtx


  return std::move(outputVertices);
}
