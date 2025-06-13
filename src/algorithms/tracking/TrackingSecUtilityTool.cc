#include "TrackingSecUtilityTool.h"

#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Vertexing/ImpactPointEstimator.hpp"

#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/GenericParticleHypothesis.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#if Acts_VERSION_MAJOR >= 32
#include <Acts/Propagator/VoidNavigator.hpp>
#else
#include <Acts/Propagator/detail/VoidPropagatorComponents.hpp>
#endif
#include <Acts/Utilities/Result.hpp>
#include <Acts/Utilities/VectorHelpers.hpp>
#include <Acts/Vertexing/FullBilloirVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include "Acts/Vertexing/TrackDensityVertexFinder.hpp"
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
#include <algorithms/algorithm.h>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov4f.h>
#include <math.h>
#include <optional>
#include <tuple>
#include <utility>

#include "edm4eic/ReconstructedParticle.h"
#include "extensions/spdlog/SpdlogToActs.h"

eicrecon::TrackingSecUtilityTool::TrackingSecUtilityTool() {}
eicrecon::TrackingSecUtilityTool::~TrackingSecUtilityTool() {}

void eicrecon::TrackingSecUtilityTool::write2screen() {
  std::cout << "!!!This is just an absolute test mate!!!\n";
}
std::unique_ptr<edm4eic::VertexCollection> eicrecon::TrackingSecUtilityTool::calcPrimaryVtx(
    const edm4eic::ReconstructedParticleCollection* reconParticles,
    std::vector<const ActsExamples::Trajectories*> trajectories,
    Acts::AdaptiveMultiVertexFinder& vtxfinder, Acts::VertexingOptions vtxOpts,
    Acts::AdaptiveMultiVertexFinder::Config& vtxfinderCfg, Acts::IVertexFinder::State& state) {

  std::cout << "USA!!! USA!!!! USA!!! HURRRRAAAAHHHH!!!!\n";
  /*
  auto outprmVertices = std::make_unique<edm4eic::VertexCollection>();
  //Input track trajectories
#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::InputTrack> inTracks;
#else
  std::vector<const Acts::BoundTrackParameters*> inTrackPointersSecondary;
#endif

  //Two Track Vertex fitting
  for (const auto trajectory : trajectories){
    auto tips = trajectory->tips();
    if (tips.empty()){
      continue;
    }
    std::cout << "PrimaryVertex: Tip size: " << tips.size() << " tips" << std::endl;
    //std::abort();
    /// CKF can provide multiple track trajectories for a single input seed
    for (auto& tip : tips){
      ActsExamples::TrackParameters par = trajectory->trackParameters(tip);

#if Acts_VERSION_MAJOR >= 33
      inputTracks.emplace_back(&(trajectory->trackParameters(tip)));
#else
      inputTrackPointers.push_back(&(trajectory->trackParameters(tip)));
#endif
      m_log->trace("Track local position at input = {} mm, {} mm", par.localPosition().x() / Acts::UnitConstants::mm, par.localPosition().y() / Acts::UnitConstants::mm);
    }
  } // end of trajectories for-loop

#if Acts_VERSION_MAJOR >= 33
  std::vector<Acts::Vertex> vertices;
  auto result = vtxfinder.find(inputTracks, vtxOpts, state);
#else
  std::vector<Acts::Vertex<Acts::BoundTrackParameters>> vertices;
  auto result = vtxfinder.find(inputTrackPointers, vtxOpts, state);
#endif
  if (result.ok()) {
    vertices = std::move(result.value());
  }

  for (const auto& vtx : vertices) {                                                                                        edm4eic::Cov4f cov(vtx.fullCovariance()(0,0), vtx.fullCovariance()(1,1), vtx.fullCovariance()(2,2), vtx.fullCovariance()(3,3),
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
      const auto& par = vtxfinderCfg.extractParameters(t.originalParams);
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
*/
  return 0;
}
