// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "IterativeVertexFinder.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Utilities/Helpers.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Vertexing/FullBilloirVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/IterativeVertexFinder.hpp>
#include <Acts/Vertexing/LinearizedTrack.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexFinderConcept.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/ZScanVertexFinder.hpp>

#include <edm4eic/Cov3f.h>
#include <edm4eic/Vertex.h>

#include "extensions/spdlog/SpdlogFormatters.h"
#include "extensions/spdlog/SpdlogToActs.h"

#include <TDatabasePDG.h>
#include <tuple>

void eicrecon::IterativeVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log = log;

  m_geoSvc = geo_svc;

  m_BField =
      std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
  m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);
}

std::unique_ptr<edm4eic::VertexCollection> eicrecon::IterativeVertexFinder::produce(
    std::vector<const ActsExamples::Trajectories*> trajectories) {

  auto outputVertices = std::make_unique<edm4eic::VertexCollection>();

  using Propagator        = Acts::Propagator<Acts::EigenStepper<>>;
  using PropagatorOptions = Acts::PropagatorOptions<>;
  using Linearizer        = Acts::HelicalTrackLinearizer<Propagator>;
  using VertexFitter      = Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, Linearizer>;
  using ImpactPointEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, Propagator>;
  using VertexSeeder         = Acts::ZScanVertexFinder<VertexFitter>;
  using VertexFinder         = Acts::IterativeVertexFinder<VertexFitter, VertexSeeder>;
  using VertexFinderOptions  = Acts::VertexingOptions<Acts::BoundTrackParameters>;

  Acts::EigenStepper<> stepper(m_BField);
  auto propagator = std::make_shared<Propagator>(stepper);
  auto logLevel   = eicrecon::SpdlogToActsLevel(m_geoSvc->getActsRelatedLogger()->level());

  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("CKFTracking Logger", logLevel));
  Acts::PropagatorOptions opts(m_geoctx, m_fieldctx, Acts::LoggerWrapper{logger()});

  // Setup the vertex fitter
  VertexFitter::Config vertexFitterCfg;
  VertexFitter vertexFitter(vertexFitterCfg);
  // Setup the track linearizer
  Linearizer::Config linearizerCfg(m_BField, propagator);
  Linearizer linearizer(linearizerCfg);
  // Setup the seed finder
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagator);
  ImpactPointEstimator ipEst(ipEstCfg);
  VertexSeeder::Config seederCfg(ipEst);
  VertexSeeder seeder(seederCfg);
  // Set up the actual vertex finder
  VertexFinder::Config finderCfg(vertexFitter, linearizer, std::move(seeder), ipEst);
  finderCfg.maxVertices                 = m_cfg.m_maxVertices;
  finderCfg.reassignTracksAfterFirstFit = m_cfg.m_reassignTracksAfterFirstFit;
  VertexFinder finder(finderCfg);
  VertexFinder::State state(*m_BField, m_fieldctx);
  VertexFinderOptions finderOpts(m_geoctx, m_fieldctx);

  std::vector<const Acts::BoundTrackParameters*> inputTrackPointers;

  for (const auto& trajectory : trajectories) {
    auto tips = trajectory->tips();
    if (tips.empty()) {
      continue;
    }
    /// CKF can provide multiple track trajectories for a single input seed
    for (auto& tip : tips) {
      inputTrackPointers.push_back(&(trajectory->trackParameters(tip)));
    }
  }

  std::vector<Acts::Vertex<Acts::BoundTrackParameters>> vertices;
  auto result = finder.find(inputTrackPointers, finderOpts, state);
  if (result.ok()) {
    vertices = std::move(result.value());
  }

  for (const auto& vtx : vertices) {
    edm4eic::Cov3f cov(vtx.covariance()(0, 0), vtx.covariance()(1, 1), vtx.covariance()(2, 2),
                       vtx.covariance()(0, 1), vtx.covariance()(0, 2), vtx.covariance()(1, 2));

    auto eicvertex = outputVertices->create();
    eicvertex.setPrimary(1);                                  // boolean flag if vertex is primary vertex of event
    eicvertex.setChi2((float)vtx.fitQuality().first);         // chi2
    eicvertex.setProbability((float)vtx.fitQuality().second); // ndf
    eicvertex.setPosition({
         (float)vtx.position().x(),
         (float)vtx.position().y(),
         (float)vtx.position().z()
    }); // vtxposition
    eicvertex.setPositionError(cov);                          // covariance
    eicvertex.setAlgorithmType(1);                            // algorithmtype
    eicvertex.setTime((float)vtx.time());                     // time
  }

  return std::move(outputVertices);
}
