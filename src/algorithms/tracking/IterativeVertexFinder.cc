// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "IterativeVertexFinder.h"

#include <Acts/Definitions/Units.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/VoidNavigator.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Vertexing/FullBilloirVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/IVertexFinder.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/IterativeVertexFinder.hpp>
#include <Acts/Vertexing/LinearizedTrack.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/ZScanVertexFinder.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/Cov4f.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector4f.h>
#include <fmt/format.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <cmath>
#include <string>
#include <utility>

#include "extensions/spdlog/SpdlogToActs.h"

void eicrecon::IterativeVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log = log;

  m_geoSvc = geo_svc;

  m_BField   = m_geoSvc->getFieldProvider();
  m_fieldctx = Acts::MagneticFieldContext{};
}

std::unique_ptr<edm4eic::VertexCollection> eicrecon::IterativeVertexFinder::produce(
    std::vector<const ActsExamples::Trajectories*> trajectories,
    const edm4eic::ReconstructedParticleCollection* reconParticles) {

  auto outputVertices = std::make_unique<edm4eic::VertexCollection>();

  using Propagator           = Acts::Propagator<Acts::EigenStepper<>>;
  using Linearizer           = Acts::HelicalTrackLinearizer;
  using VertexFitter         = Acts::FullBilloirVertexFitter;
  using ImpactPointEstimator = Acts::ImpactPointEstimator;
  using VertexSeeder         = Acts::ZScanVertexFinder;
  using VertexFinder         = Acts::IterativeVertexFinder;
  using VertexFinderOptions  = Acts::VertexingOptions;

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("IVF", m_log));

  Acts::EigenStepper<> stepper(m_BField);

  // Set up propagator with void navigator
  auto propagator = std::make_shared<Propagator>(stepper, Acts::VoidNavigator{},
                                                 logger().cloneWithSuffix("Prop"));

  // Setup the track linearizer
  Linearizer::Config linearizerCfg;
  linearizerCfg.bField     = m_BField;
  linearizerCfg.propagator = propagator;
  Linearizer linearizer(linearizerCfg, logger().cloneWithSuffix("HelLin"));

  // Setup the vertex fitter
  VertexFitter::Config vertexFitterCfg;
  vertexFitterCfg.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  vertexFitterCfg.trackLinearizer.connect<&Linearizer::linearizeTrack>(&linearizer);
  VertexFitter vertexFitter(vertexFitterCfg);

  // Setup the seed finder
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagator);
  ImpactPointEstimator ipEst(ipEstCfg);
  VertexSeeder::Config seederCfg(ipEst);
  seederCfg.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  auto seeder = std::make_shared<VertexSeeder>(seederCfg);

  // Set up the actual vertex finder
  VertexFinder::Config finderCfg(std::move(vertexFitter), std::move(seeder), std::move(ipEst));
  finderCfg.maxVertices                 = m_cfg.maxVertices;
  finderCfg.reassignTracksAfterFirstFit = m_cfg.reassignTracksAfterFirstFit;
  finderCfg.extractParameters.connect<&Acts::InputTrack::extractParameters>();
  finderCfg.trackLinearizer.connect<&Linearizer::linearizeTrack>(&linearizer);
  finderCfg.field = m_BField;
  VertexFinder finder(std::move(finderCfg));
  Acts::IVertexFinder::State state(std::in_place_type<VertexFinder::State>, *m_BField, m_fieldctx);
  VertexFinderOptions finderOpts(m_geoctx, m_fieldctx);

  std::vector<Acts::InputTrack> inputTracks;

  for (const auto& trajectory : trajectories) {
    auto tips = trajectory->tips();
    if (tips.empty()) {
      continue;
    }
    /// CKF can provide multiple track trajectories for a single input seed
    for (auto& tip : tips) {
      ActsExamples::TrackParameters par = trajectory->trackParameters(tip);

      inputTracks.emplace_back(&(trajectory->trackParameters(tip)));
      m_log->trace("Track local position at input = {} mm, {} mm",
                   par.localPosition().x() / Acts::UnitConstants::mm,
                   par.localPosition().y() / Acts::UnitConstants::mm);
    }
  }

  std::vector<Acts::Vertex> vertices;
  auto result = finder.find(inputTracks, finderOpts, state);
  if (result.ok()) {
    vertices = std::move(result.value());
  }

  for (const auto& vtx : vertices) {
    edm4eic::Cov4f cov(vtx.fullCovariance()(0, 0), vtx.fullCovariance()(1, 1),
                       vtx.fullCovariance()(2, 2), vtx.fullCovariance()(3, 3),
                       vtx.fullCovariance()(0, 1), vtx.fullCovariance()(0, 2),
                       vtx.fullCovariance()(0, 3), vtx.fullCovariance()(1, 2),
                       vtx.fullCovariance()(1, 3), vtx.fullCovariance()(2, 3));
    auto eicvertex = outputVertices->create();
    eicvertex.setType(1); // boolean flag if vertex is primary vertex of event
    eicvertex.setChi2((float)vtx.fitQuality().first); // chi2
    eicvertex.setNdf((float)vtx.fitQuality().second); // ndf
    eicvertex.setPosition({
        (float)vtx.position().x(),
        (float)vtx.position().y(),
        (float)vtx.position().z(),
        (float)vtx.time(),
    });                              // vtxposition
    eicvertex.setPositionError(cov); // covariance

    for (const auto& t : vtx.tracks()) {
      const auto& par = Acts::InputTrack::extractParameters(t.originalParams);
      m_log->trace("Track local position from vertex = {} mm, {} mm",
                   par.localPosition().x() / Acts::UnitConstants::mm,
                   par.localPosition().y() / Acts::UnitConstants::mm);
      float loc_a = par.localPosition().x();
      float loc_b = par.localPosition().y();

      for (const auto& part : *reconParticles) {
        const auto& tracks = part.getTracks();
        for (const auto& trk : tracks) {
          const auto& traj    = trk.getTrajectory();
          const auto& trkPars = traj.getTrackParameters();
          for (const auto& par : trkPars) {
            const double EPSILON = 1.0e-4; // mm
            if (std::abs((par.getLoc().a / edm4eic::unit::mm) - (loc_a / Acts::UnitConstants::mm)) <
                    EPSILON &&
                std::abs((par.getLoc().b / edm4eic::unit::mm) - (loc_b / Acts::UnitConstants::mm)) <
                    EPSILON) {
              m_log->trace(
                  "From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
                  par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
              eicvertex.addToAssociatedParticles(part);
            } // endif
          } // end for par
        } // end for trk
      } // end for part
    } // end for t
    m_log->debug("One vertex found at (x,y,z) = ({}, {}, {}) mm.",
                 vtx.position().x() / Acts::UnitConstants::mm,
                 vtx.position().y() / Acts::UnitConstants::mm,
                 vtx.position().z() / Acts::UnitConstants::mm);

  } // end for vtx

  return outputVertices;
}
