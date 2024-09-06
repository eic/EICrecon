// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "SecondaryVertexFinder.h"

#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/GenericParticleHypothesis.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/detail/VoidPropagatorComponents.hpp>
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
#include "extensions/spdlog/SpdlogToActs.h"

void eicrecon::SecondaryVertexFinder::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                           std::shared_ptr<spdlog::logger> log) {

  m_log = log;

  m_geoSvc = geo_svc;

  m_BField =
      std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
  m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);
}

std::unique_ptr<edm4eic::VertexCollection> eicrecon::SecondaryVertexFinder::produce(
    std::vector<const edm4eic::Track*> track,std::vector<const ActsExamples::Trajectories*> trajectories) {

  auto primaryVertices = std::make_unique<edm4eic::VertexCollection>();
  auto outputVertices  = std::make_unique<edm4eic::VertexCollection>();

//   using Propagator        = Acts::Propagator<Acts::EigenStepper<>>;
//   using PropagatorOptions = Acts::PropagatorOptions<>;
//   using Linearizer        = Acts::HelicalTrackLinearizer<Propagator>;
//   using VertexFitter      = Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, Linearizer>;
//   using ImpactPointEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, Propagator>;
//   using VertexSeeder         = Acts::ZScanVertexFinder<VertexFitter>;
//   using VertexFinder         = Acts::IterativeVertexFinder<VertexFitter, VertexSeeder>;
//   using VertexFinderOptions  = Acts::VertexingOptions<Acts::BoundTrackParameters>;

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("SVF", m_log));

//   Acts::EigenStepper<> stepper(m_BField);

//   // Set up propagator with void navigator
//   auto propagator = std::make_shared<Propagator>(stepper, Acts::detail::VoidNavigator{},
//                                                  logger().cloneWithSuffix("Prop"));
//   Acts::PropagatorOptions opts(m_geoctx, m_fieldctx);

//   // Setup the vertex fitter
//   VertexFitter::Config vertexFitterCfg;
//   VertexFitter vertexFitter(vertexFitterCfg);
//   // Setup the track linearizer
//   Linearizer::Config linearizerCfg(m_BField, propagator);
//   Linearizer linearizer(linearizerCfg, logger().cloneWithSuffix("HelLin"));
//   // Setup the seed finder
//   ImpactPointEstimator::Config ipEstCfg(m_BField, propagator);
//   ImpactPointEstimator ipEst(ipEstCfg);
//   VertexSeeder::Config seederCfg(ipEst);
//   VertexSeeder seeder(seederCfg);
//   // Set up the actual vertex finder
//   VertexFinder::Config finderCfg(std::move(vertexFitter), std::move(linearizer), std::move(seeder),
//                                  std::move(ipEst));
//   finderCfg.maxVertices                 = m_cfg.maxVertices;
//   finderCfg.reassignTracksAfterFirstFit = m_cfg.reassignTracksAfterFirstFit;
// #if Acts_VERSION_MAJOR >= 31
//   VertexFinder finder(std::move(finderCfg));
// #else
//   VertexFinder finder(finderCfg);
// #endif
//   VertexFinder::State state(*m_BField, m_fieldctx);
//   VertexFinderOptions finderOpts(m_geoctx, m_fieldctx);

//   std::vector<const Acts::BoundTrackParameters*> inputTrackPointers;

//   for (const auto& trajectory : trajectories) {
//     auto tips = trajectory->tips();
//     if (tips.empty()) {
//       continue;
//     }
//     /// CKF can provide multiple track trajectories for a single input seed
//     for (auto& tip : tips) {
//       inputTrackPointers.push_back(&(trajectory->trackParameters(tip)));
//     }
//   }

//   std::vector<Acts::Vertex<Acts::BoundTrackParameters>> vertices;
//   auto result = finder.find(inputTrackPointers, finderOpts, state);
//   if (result.ok()) {
//     vertices = std::move(result.value());
//   }

//   for (const auto& vtx : vertices) {
//     edm4eic::Cov4f cov(vtx.fullCovariance()(0, 0), vtx.fullCovariance()(1, 1),
//                        vtx.fullCovariance()(2, 2), vtx.fullCovariance()(3, 3),
//                        vtx.fullCovariance()(0, 1), vtx.fullCovariance()(0, 2),
//                        vtx.fullCovariance()(0, 3), vtx.fullCovariance()(1, 2),
//                        vtx.fullCovariance()(1, 3), vtx.fullCovariance()(2, 3));
//     auto eicvertex = primaryVertices->create();
//     eicvertex.setType(1); // boolean flag if vertex is primary vertex of event
//     eicvertex.setChi2((float)vtx.fitQuality().first); // chi2
//     eicvertex.setNdf((float)vtx.fitQuality().second); // ndf
//     eicvertex.setPosition({
//         (float)vtx.position().x(),
//         (float)vtx.position().y(),
//         (float)vtx.position().z(),
//         (float)vtx.time(),
//     });                              // vtxposition
//     eicvertex.setPositionError(cov); // covariance
//   }

  // Set-up EigenStepper
  Acts::EigenStepper<> stepperSec(m_BField);

  // Set-up the propagator
  using PropagatorSec        = Acts::Propagator<Acts::EigenStepper<>>;
  using PropagatorOptionsSec = Acts::PropagatorOptions<>;

  // Set up propagator with void navigator
  auto propagatorSec = std::make_shared<PropagatorSec>(stepperSec, Acts::detail::VoidNavigator{},
                                                 logger().cloneWithSuffix("PropSec"));
  // Acts::PropagatorOptions optsSec(m_geoctx, m_fieldctx);

  //set up Impact estimator
  using ImpactPointEstimator = Acts::ImpactPointEstimator<Acts::BoundTrackParameters, PropagatorSec>;
  ImpactPointEstimator::Config ipEstCfg(m_BField, propagatorSec);
  ImpactPointEstimator ipEst(ipEstCfg);

  // using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  using LinearizerSec = Acts::HelicalTrackLinearizer<PropagatorSec>;
  // Setup the track linearizer
  LinearizerSec::Config linearizerCfgSec(m_BField, propagatorSec);
  LinearizerSec linearizerSec(linearizerCfgSec, logger().cloneWithSuffix("HelLinSec"));

  //using VertexFitterSec = Acts::FullBilloirVertexFitter<Acts::BoundTrackParameters, LinearizerSec>;
  //Trying multivertex fitter here...
  using VertexFitterSec = Acts::AdaptiveMultiVertexFitter<Acts::BoundTrackParameters, LinearizerSec>;
  // Setup the vertex fitter
  VertexFitterSec::Config vertexFitterCfgSec(ipEst);
  VertexFitterSec vertexFitterSec(vertexFitterCfgSec);

  // Set up the vertex seed finder
  using VertexSeederSec = Acts::TrackDensityVertexFinder<VertexFitterSec, Acts::GaussianTrackDensity<Acts::BoundTrackParameters>>;
  VertexSeederSec seeder;

  // The vertex finder type
  using VertexFinderSec = Acts::AdaptiveMultiVertexFinder<VertexFitterSec, VertexSeederSec>;

  VertexFinderSec::Config vertexfinderCfgSec(std::move(vertexFitterSec), seeder, ipEst,
                              std::move(linearizerSec), m_BField);
  // We do not want to use a beamspot constraint here
  //vertexfinderCfgSec.useBeamSpotConstraint = false;
  vertexfinderCfgSec.tracksMaxZinterval = 1. * Acts::UnitConstants::mm;

  // Instantiate the finder
  VertexFinderSec vertexfinderSec(std::move(vertexfinderCfgSec),logger().clone());
  // The vertex finder state
  typename VertexFinderSec::State stateSec;

  // Acts::MagneticFieldProvider::Cache fieldCache = m_BField->makeCache(m_fieldctx);
  //---->VertexFitterSec::State stateSec(m_BField->makeCache(m_fieldctx));

  using VertexFinderOptionsSec = Acts::VertexingOptions<Acts::BoundTrackParameters>;
  VertexFinderOptionsSec vfOptions(m_geoctx, m_fieldctx);
  // auto bCache = m_magneticField->makeCache(m_fieldctx);

  std::vector<const Acts::BoundTrackParameters*> inputTrackPointersSecondary;

  //checking the track start vertex
  //if(edm4eic::ReconstructedParticle().getStartVertex().getPosition().x)
    //std::cout<<"Start vertex of the track: "<<track::getPosition().z<<std::endl;
    std::cout<<"Start vertex of the track: "<<track.size()<<std::endl;//getPosition().z<<std::endl;

  for (std::vector<Acts::BoundTrackParameters>::size_type i = 0; i != trajectories.size(); i++) {
    // trajectories[i].doSomething();
    auto tips = trajectories[i]->tips();
    if (tips.empty()) {
      continue;
    }
    std::cout << "Trajectory i: " << i << " has " << tips.size() << " tips" << std::endl;
    for (std::vector<Acts::BoundTrackParameters>::size_type j = i + 1; j != trajectories.size();
         j++) {
      auto tips2 = trajectories[j]->tips();
      if (tips2.empty()) {
        continue;
      }
      std::cout << "Trajectory j: " << j << " has " << tips2.size() << " tips" << std::endl;
      for (auto& tip : tips) {
        inputTrackPointersSecondary.push_back(&(trajectories[i]->trackParameters(tip)));
        //print track parameters:
        // const auto& boundParam = trajectories[i]->trackParameters(tip);
        //     const auto& parameter  = boundParam.parameters();
        //     std::cout << "\ti Track parameters: " << std::endl; 
        //     std::cout << "\ti Loc0: " << parameter[Acts::eBoundLoc0] << std::endl;
        //     std::cout << "\ti Loc1: " << parameter[Acts::eBoundLoc1] << std::endl;
        //     std::cout << "\ti Theta: " << parameter[Acts::eBoundTheta] << std::endl;
        //     std::cout << "\ti Phi: " << parameter[Acts::eBoundPhi] << std::endl;
        //     std::cout << "\ti QOverP: " << parameter[Acts::eBoundQOverP] << std::endl;
        //     std::cout << "\ti Time: " << parameter[Acts::eBoundTime] << std::endl;
      }
      for (auto& tip : tips2) {
        inputTrackPointersSecondary.push_back(&(trajectories[j]->trackParameters(tip)));
        //print track parameters:
        // const auto& boundParam = trajectories[j]->trackParameters(tip);
        //     const auto& parameter  = boundParam.parameters();
        //     std::cout << "\tj Track parameters: " << std::endl; 
        //     std::cout << "\tj Loc0: " << parameter[Acts::eBoundLoc0] << std::endl;
        //     std::cout << "\tj Loc1: " << parameter[Acts::eBoundLoc1] << std::endl;
        //     std::cout << "\tj Theta: " << parameter[Acts::eBoundTheta] << std::endl;
        //     std::cout << "\tj Phi: " << parameter[Acts::eBoundPhi] << std::endl;
        //     std::cout << "\tj QOverP: " << parameter[Acts::eBoundQOverP] << std::endl;
        //     std::cout << "\tj Time: " << parameter[Acts::eBoundTime] << std::endl;
      }
      // run the vertex finder for both tracks
      // std::vector<Acts::Vertex<Acts::BoundTrackParameters>> verticesSecondary;
      // std::vector<Acts::Vertex> verticesSecondary;
      // auto resultSecondary = finder.find(inputTrackPointersSecondary, finderOpts, state);
      std::cout << "Fitting secondary vertex" << std::endl;
      auto resultSecondary =
          //vertexFitterSec.fit(inputTrackPointersSecondary, linearizerSec, vfOptions, stateSec);
          vertexfinderSec.find(inputTrackPointersSecondary, vfOptions, stateSec);
      std::cout << "Secondary vertex fit done" << std::endl;
      // auto resultSecondary = vertexFitter.fit(inputTrackPointersSecondary, vfOptions,
      // fieldCache);

      // Acts::Result<Acts::Vertex> Acts::FullBilloirVertexFitter::fit(
      //     const std::vector<InputTrack>& paramVector,
      //     const VertexingOptions& vertexingOptions,
      //     MagneticFieldProvider::Cache& fieldCache)
      if (resultSecondary.ok()) {
        std::cout << "Secondary vertex fit succeeded" << std::endl;
        auto secvertex = resultSecondary.value();
        // verticesSecondary = std::move(resultSecondary.value());
        // }
/*
        // for (const auto& vtx : verticesSecondary) {
        edm4eic::Cov4f cov(secvertex.fullCovariance()(0, 0), secvertex.fullCovariance()(1, 1),
                           secvertex.fullCovariance()(2, 2), secvertex.fullCovariance()(3, 3),
                           secvertex.fullCovariance()(0, 1), secvertex.fullCovariance()(0, 2),
                           secvertex.fullCovariance()(0, 3), secvertex.fullCovariance()(1, 2),
                           secvertex.fullCovariance()(1, 3), secvertex.fullCovariance()(2, 3));
        auto eicvertex = outputVertices->create();
        eicvertex.setType(0); // boolean flag if vertex is primary vertex of event
        eicvertex.setChi2((float)secvertex.fitQuality().first); // chi2
        eicvertex.setNdf((float)secvertex.fitQuality().second); // ndf
        eicvertex.setPosition({
            (float)secvertex.position().x(),
            (float)secvertex.position().y(),
            (float)secvertex.position().z(),
            (float)secvertex.time(),
        });                              // vtxposition
        eicvertex.setPositionError(cov); // covariance
*/
        // }
      } else {
        std::cout << "Secondary vertex fit failed" << std::endl;
      }
      // empty the vector for the next set of tracks
      inputTrackPointersSecondary.clear();
    }
  }

  return std::move(outputVertices);
}
