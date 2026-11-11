// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#include "SecondaryVertexFinder.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#if Acts_VERSION_MAJOR >= 46
#include <Acts/EventData/BoundTrackParameters.hpp>
#else
#include <Acts/EventData/TrackParameters.hpp>
#endif
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/VoidNavigator.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Utilities/AnnealingUtility.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveGridTrackDensity.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
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
#include <edm4hep/Vector3f.h>
#include <edm4hep/Vector4f.h>
#include <edm4hep/utils/vector_utils.h>
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
  
  // Note: runs through the vertices per event
  for (const auto& vtx : vertices) {
    // Note: Fill information of one vertex
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

    // Note: Tracks associated to the vertex are compared with reconstructed particles!
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
              // trace("From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
              //       trkPar.getLoc().a / edm4eic::unit::mm, trkPar.getLoc().b / edm4eic::unit::mm);
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

void SecondaryVertexFinder::storeSecondaryVertices(
    const std::vector<Acts::Vertex>& primaryVertices,
    const std::vector<Acts::Vertex>& secondaryVertices,
    const edm4eic::ReconstructedParticleCollection& reconParticles,
    edm4eic::VertexCollection& outputVertices,
    Acts::MagneticFieldContext mctx,
    Acts::GeometryContext gctx,
    Acts::ImpactPointEstimator::State& ImPoEs_state,
    Acts::ImpactPointEstimator& ipEst,
    int vertexType) const {
  
  for (const auto& pv : primaryVertices) {
    edm4hep::Vector3f pvCoordinates{static_cast<float>(pv.position().x()), 
                                    static_cast<float>(pv.position().y()), 
                                    static_cast<float>(pv.position().z())};

    // trace("PV=({},{},{})",static_cast<float>(pv.position().x()), 
    //                       static_cast<float>(pv.position().y()), 
    //                       static_cast<float>(pv.position().z()));

    for (const auto& vtx : secondaryVertices) {
      std::vector<edm4eic::ReconstructedParticle> daughters;

      // Note: parameters and covariances of Acts tracks have to be set here!
      // Acts::BoundVector paramsDaughterA;
      // Acts::BoundVector paramsDaughterB;

      Acts::BoundTrackParameters paramsTrackMatchedToRecoA;
      Acts::BoundTrackParameters paramsTrackMatchedToRecoB;

      // edm4eic::Cov6f daughterACov;
      // edm4eic::Cov6f daughterBCov;

      int dtr_counter = 0;

      for (const auto& t : vtx.tracks()) {
        const auto par = Acts::InputTrack::extractParameters(t.originalParams);

        // trace("Track local position from vertex = {} mm, {} mm",
        //       par.localPosition().x() / Acts::UnitConstants::mm,
        //       par.localPosition().y() / Acts::UnitConstants::mm);
        
        float loc_a = par.localPosition().x();
        float loc_b = par.localPosition().y();

        for (const auto& part : reconParticles) {
          const auto& tracks = part.getTracks();
          for (const auto& trk : tracks) {
            const auto& traj    = trk.getTrajectory();
            const auto& trkPars = traj.getTrackParameters();
            for (const auto& trkPar : trkPars) {
              const double EPSILON = 1.0e-4; // mm
              
              if (std::abs((trkPar.getLoc().a / edm4eic::unit::mm) -
                          (loc_a / Acts::UnitConstants::mm)) < EPSILON &&
                  std::abs((trkPar.getLoc().b / edm4eic::unit::mm) -
                          (loc_b / Acts::UnitConstants::mm)) < EPSILON) {

                if (daughters.size() == 0) {
                  paramsTrackMatchedToRecoA = par;
                  // daughterACov = trkPar.getCovariance();  
                  // debug("({},{})",daughterCov(0,0),daughterCov(1,1));
                  // paramsDaughterA(Acts::eBoundLoc0)   = trkPar.getLoc().a;
                  // paramsDaughterA(Acts::eBoundLoc1)   = trkPar.getLoc().b;
                  // paramsDaughterA(Acts::eBoundPhi)    = trkPar.getPhi();
                  // paramsDaughterA(Acts::eBoundTheta)  = trkPar.getTheta();
                  // paramsDaughterA(Acts::eBoundQOverP) = trkPar.getQOverP();
                  // paramsDaughterA(Acts::eBoundTime)   = 0;
                } else if (daughters.size() == 1) {
                  paramsTrackMatchedToRecoB = par;
                  // daughterBCov = trkPar.getCovariance();  
                  // paramsDaughterB(Acts::eBoundLoc0)   = trkPar.getLoc().a;
                  // paramsDaughterB(Acts::eBoundLoc1)   = trkPar.getLoc().b;
                  // paramsDaughterB(Acts::eBoundPhi)    = trkPar.getPhi();
                  // paramsDaughterB(Acts::eBoundTheta)  = trkPar.getTheta();
                  // paramsDaughterB(Acts::eBoundQOverP) = trkPar.getQOverP();
                  // paramsDaughterB(Acts::eBoundTime)   = 0;
                } else if (daughters.size() > 2) {
                  continue;
                }
                
                daughters.emplace_back(part);
              }
            }
          }
        } // End loop reco particle 
      } // End loop tracks to reco matching

      if (daughters.size() != 2) {
        daughters.clear();
        
        continue;
      }

      // Note: Fill information of one vertex
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

      // SV property
      edm4hep::Vector3f svCoordinates{static_cast<float>(vtx.position().x()), 
                                      static_cast<float>(vtx.position().y()), 
                                      static_cast<float>(vtx.position().z())};

      edm4hep::Vector3f decayLengthVector(pvCoordinates - svCoordinates);

      // Get kinematics
      edm4hep::Vector3f momentumDaughterA = daughters[0].getMomentum();
      edm4hep::Vector3f momentumDaughterB = daughters[1].getMomentum();
      edm4hep::Vector3f momentumParent    = momentumDaughterA + momentumDaughterB;

      double momentumMagDaughterA = edm4hep::utils::magnitude(momentumDaughterA);
      double momentumMagDaughterB = edm4hep::utils::magnitude(momentumDaughterB);
      double momentumMagParent    = edm4hep::utils::magnitude(momentumParent);

      float energyDaughterA = daughters[0].getEnergy();
      float energyDaughterB = daughters[1].getEnergy();
      float energyParent    = energyDaughterA + energyDaughterB;

      float invariantMassParent = std::sqrt(std::abs(energyParent * energyParent - edm4hep::utils::magnitude(momentumParent) * edm4hep::utils::magnitude(momentumParent)));

      eicvertex.setParentMomentum(momentumParent);
      eicvertex.setParentInvariantMass(invariantMassParent);
      eicvertex.setParentDecayLength(edm4hep::utils::magnitude(decayLengthVector) / edm4eic::unit::mm);
      eicvertex.setParentDca2PV(edm4hep::utils::magnitude(decayLengthVector) * std::sin(edm4hep::utils::angleBetween(pvCoordinates, svCoordinates)) / edm4eic::unit::mm);

      // Note: Acts library on action
      Acts::Vector3 pvPosition(static_cast<float>(pv.position().x()),
                               static_cast<float>(pv.position().y()),
                               static_cast<float>(pv.position().z()));

      Acts::Vector3 svPosition(static_cast<float>(vtx.position().x()),
                               static_cast<float>(vtx.position().y()),
                               static_cast<float>(vtx.position().z()));
      
      // // Note: no fancy way of setting this indeces so far
      // Acts::BoundMatrix daughterACov_IPuse = Acts::BoundMatrix::Zero();
      // Acts::BoundMatrix daughterBCov_IPuse = Acts::BoundMatrix::Zero();

      // daughterACov_IPuse(Acts::eBoundLoc0,   Acts::eBoundLoc0)   = daughterACov(0,0);
      // daughterACov_IPuse(Acts::eBoundLoc1,   Acts::eBoundLoc1)   = daughterACov(1,1);
      // daughterACov_IPuse(Acts::eBoundPhi,    Acts::eBoundPhi)    = daughterACov(2,2);
      // daughterACov_IPuse(Acts::eBoundTheta,  Acts::eBoundTheta)  = daughterACov(3,3);
      // daughterACov_IPuse(Acts::eBoundQOverP, Acts::eBoundQOverP) = daughterACov(4,4);
      // daughterACov_IPuse(Acts::eBoundTime,   Acts::eBoundTime)   = daughterACov(5,5);

      // daughterACov_IPuse(Acts::eBoundLoc0, Acts::eBoundLoc1)    = daughterACov(0,1);
      // daughterACov_IPuse(Acts::eBoundLoc0, Acts::eBoundPhi)     = daughterACov(0,2);
      // daughterACov_IPuse(Acts::eBoundLoc0, Acts::eBoundTheta)   = daughterACov(0,3);
      // daughterACov_IPuse(Acts::eBoundLoc0, Acts::eBoundQOverP)  = daughterACov(0,4);
      // daughterACov_IPuse(Acts::eBoundLoc0, Acts::eBoundTime)    = daughterACov(0,5);
      // daughterACov_IPuse(Acts::eBoundLoc1, Acts::eBoundLoc0)    = daughterACov(1,0);
      // daughterACov_IPuse(Acts::eBoundLoc1, Acts::eBoundPhi)     = daughterACov(1,2);
      // daughterACov_IPuse(Acts::eBoundLoc1, Acts::eBoundTheta)   = daughterACov(1,3);
      // daughterACov_IPuse(Acts::eBoundLoc1, Acts::eBoundQOverP)  = daughterACov(1,4);
      // daughterACov_IPuse(Acts::eBoundLoc1, Acts::eBoundTime)    = daughterACov(1,5);
      // daughterACov_IPuse(Acts::eBoundPhi, Acts::eBoundLoc0)     = daughterACov(2,0);
      // daughterACov_IPuse(Acts::eBoundPhi, Acts::eBoundLoc1)     = daughterACov(2,1);
      // daughterACov_IPuse(Acts::eBoundPhi, Acts::eBoundTheta)    = daughterACov(2,3);
      // daughterACov_IPuse(Acts::eBoundPhi, Acts::eBoundQOverP)   = daughterACov(2,4);
      // daughterACov_IPuse(Acts::eBoundPhi, Acts::eBoundTime)     = daughterACov(2,5);
      // daughterACov_IPuse(Acts::eBoundTheta, Acts::eBoundLoc0)   = daughterACov(3,0);
      // daughterACov_IPuse(Acts::eBoundTheta, Acts::eBoundLoc1)   = daughterACov(3,1);
      // daughterACov_IPuse(Acts::eBoundTheta, Acts::eBoundPhi)    = daughterACov(3,2);
      // daughterACov_IPuse(Acts::eBoundTheta, Acts::eBoundQOverP) = daughterACov(3,4);
      // daughterACov_IPuse(Acts::eBoundTheta, Acts::eBoundTime)   = daughterACov(3,5);
      // daughterACov_IPuse(Acts::eBoundQOverP, Acts::eBoundLoc0)  = daughterACov(4,0);
      // daughterACov_IPuse(Acts::eBoundQOverP, Acts::eBoundLoc1)  = daughterACov(4,1);
      // daughterACov_IPuse(Acts::eBoundQOverP, Acts::eBoundPhi)   = daughterACov(4,2);
      // daughterACov_IPuse(Acts::eBoundQOverP, Acts::eBoundTheta) = daughterACov(4,3);
      // daughterACov_IPuse(Acts::eBoundQOverP, Acts::eBoundTime)  = daughterACov(4,5);
      // daughterACov_IPuse(Acts::eBoundTime, Acts::eBoundLoc0)    = daughterACov(5,0);
      // daughterACov_IPuse(Acts::eBoundTime, Acts::eBoundLoc1)    = daughterACov(5,1);
      // daughterACov_IPuse(Acts::eBoundTime, Acts::eBoundPhi)     = daughterACov(5,2);
      // daughterACov_IPuse(Acts::eBoundTime, Acts::eBoundTheta)   = daughterACov(5,3);
      // daughterACov_IPuse(Acts::eBoundTime, Acts::eBoundQOverP)  = daughterACov(5,4);

      // daughterBCov_IPuse(Acts::eBoundLoc0,   Acts::eBoundLoc0)   = daughterBCov(0,0);
      // daughterBCov_IPuse(Acts::eBoundLoc1,   Acts::eBoundLoc1)   = daughterBCov(1,1);
      // daughterBCov_IPuse(Acts::eBoundPhi,    Acts::eBoundPhi)    = daughterBCov(2,2);
      // daughterBCov_IPuse(Acts::eBoundTheta,  Acts::eBoundTheta)  = daughterBCov(3,3);
      // daughterBCov_IPuse(Acts::eBoundQOverP, Acts::eBoundQOverP) = daughterBCov(4,4);
      // daughterBCov_IPuse(Acts::eBoundTime,   Acts::eBoundTime)   = daughterBCov(5,5);

      // daughterBCov_IPuse(Acts::eBoundLoc0, Acts::eBoundLoc1)    = daughterBCov(0,1);
      // daughterBCov_IPuse(Acts::eBoundLoc0, Acts::eBoundPhi)     = daughterBCov(0,2);
      // daughterBCov_IPuse(Acts::eBoundLoc0, Acts::eBoundTheta)   = daughterBCov(0,3);
      // daughterBCov_IPuse(Acts::eBoundLoc0, Acts::eBoundQOverP)  = daughterBCov(0,4);
      // daughterBCov_IPuse(Acts::eBoundLoc0, Acts::eBoundTime)    = daughterBCov(0,5);
      // daughterBCov_IPuse(Acts::eBoundLoc1, Acts::eBoundLoc0)    = daughterBCov(1,0);
      // daughterBCov_IPuse(Acts::eBoundLoc1, Acts::eBoundPhi)     = daughterBCov(1,2);
      // daughterBCov_IPuse(Acts::eBoundLoc1, Acts::eBoundTheta)   = daughterBCov(1,3);
      // daughterBCov_IPuse(Acts::eBoundLoc1, Acts::eBoundQOverP)  = daughterBCov(1,4);
      // daughterBCov_IPuse(Acts::eBoundLoc1, Acts::eBoundTime)    = daughterBCov(1,5);
      // daughterBCov_IPuse(Acts::eBoundPhi, Acts::eBoundLoc0)     = daughterBCov(2,0);
      // daughterBCov_IPuse(Acts::eBoundPhi, Acts::eBoundLoc1)     = daughterBCov(2,1);
      // daughterBCov_IPuse(Acts::eBoundPhi, Acts::eBoundTheta)    = daughterBCov(2,3);
      // daughterBCov_IPuse(Acts::eBoundPhi, Acts::eBoundQOverP)   = daughterBCov(2,4);
      // daughterBCov_IPuse(Acts::eBoundPhi, Acts::eBoundTime)     = daughterBCov(2,5);
      // daughterBCov_IPuse(Acts::eBoundTheta, Acts::eBoundLoc0)   = daughterBCov(3,0);
      // daughterBCov_IPuse(Acts::eBoundTheta, Acts::eBoundLoc1)   = daughterBCov(3,1);
      // daughterBCov_IPuse(Acts::eBoundTheta, Acts::eBoundPhi)    = daughterBCov(3,2);
      // daughterBCov_IPuse(Acts::eBoundTheta, Acts::eBoundQOverP) = daughterBCov(3,4);
      // daughterBCov_IPuse(Acts::eBoundTheta, Acts::eBoundTime)   = daughterBCov(3,5);
      // daughterBCov_IPuse(Acts::eBoundQOverP, Acts::eBoundLoc0)  = daughterBCov(4,0);
      // daughterBCov_IPuse(Acts::eBoundQOverP, Acts::eBoundLoc1)  = daughterBCov(4,1);
      // daughterBCov_IPuse(Acts::eBoundQOverP, Acts::eBoundPhi)   = daughterBCov(4,2);
      // daughterBCov_IPuse(Acts::eBoundQOverP, Acts::eBoundTheta) = daughterBCov(4,3);
      // daughterBCov_IPuse(Acts::eBoundQOverP, Acts::eBoundTime)  = daughterBCov(4,5);
      // daughterBCov_IPuse(Acts::eBoundTime, Acts::eBoundLoc0)    = daughterBCov(5,0);
      // daughterBCov_IPuse(Acts::eBoundTime, Acts::eBoundLoc1)    = daughterBCov(5,1);
      // daughterBCov_IPuse(Acts::eBoundTime, Acts::eBoundPhi)     = daughterBCov(5,2);
      // daughterBCov_IPuse(Acts::eBoundTime, Acts::eBoundTheta)   = daughterBCov(5,3);
      // daughterBCov_IPuse(Acts::eBoundTime, Acts::eBoundQOverP)  = daughterBCov(5,4);

      // // Define Perigee surface at which reconstructed track parameters are set
      // // Note: this part might require a heavy overhaul, for now we just want to make it work!
      // auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0,0,0));
      
      // // ToDo: fix particle hypothesis adn reference surface
      // // Particleypothesis is given in the extracted boundtrkparams!
      // Acts::BoundTrackParameters parametersTrackA(perigee, paramsDaughterA, daughterACov_IPuse, Acts::ParticleHypothesis::pion());
      // Acts::BoundTrackParameters parametersTrackB(perigee, paramsDaughterB, daughterBCov_IPuse, Acts::ParticleHypothesis::pion());

      // Then use the IP estimator to get all the information you need!
      // Note pvPosition might not be in the coordinate system we need.
      auto resultA = ipEst.estimate3DImpactParameters(gctx, mctx, paramsTrackMatchedToRecoA, pvPosition, ImPoEs_state);
      auto resultB = ipEst.estimate3DImpactParameters(gctx, mctx, paramsTrackMatchedToRecoB, pvPosition, ImPoEs_state);

      double daughtersDCA2PV[] = {0, 0};

      if(resultA.ok()){
              Acts::BoundTrackParameters trk_boundpar_vtx_A = resultA.value();
              const auto& trk_vtx_params_A = trk_boundpar_vtx_A.parameters();
              
              daughtersDCA2PV[0] = std::hypot(trk_vtx_params_A[Acts::eBoundLoc0], trk_vtx_params_A[Acts::eBoundLoc1]);
      }
      if(resultB.ok()){
              Acts::BoundTrackParameters trk_boundpar_vtx_B = resultB.value();
              const auto& trk_vtx_params_B = trk_boundpar_vtx_B.parameters();
              
              daughtersDCA2PV[1] = std::hypot(trk_vtx_params_B[Acts::eBoundLoc0], trk_vtx_params_B[Acts::eBoundLoc1]);
      }

      int counter = 0;
      for (const auto& daughter : daughters) {
        eicvertex.addToAssociatedParticles(daughter);
        eicvertex.addToDaughterMomentum(daughter.getMomentum());
        eicvertex.addToDaughterPDG(daughter.getPDG());
        eicvertex.addToDaughterDca2PV(daughtersDCA2PV[counter]);
        // eicvertex.addToDaughterDca2PVError();
        // eicvertex.addToDaughterPairIndices();
        counter++;
      }

      daughters.clear();
    } // End loop SV
  } // End loop PV

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

  // Set magnetic field cache (need for IPs estimation)
  Acts::ImpactPointEstimator::State ImPoEs_state;
  Acts::MagneticFieldProvider::Cache field_cache = m_BField->makeCache(mctx);
  
  ImPoEs_state.fieldCache = field_cache;

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
    // trace("Track local position at input = {} mm, {} mm",
    //       track.parameters()[Acts::eBoundLoc0] / Acts::UnitConstants::mm,
    //       track.parameters()[Acts::eBoundLoc1] / Acts::UnitConstants::mm);
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

    inputTracks.clear();
    vertices.clear();
  } else {
    // Secondary vertex mode: run AMVF on all pairwise track combinations
    VertexFinder finder(std::move(vertexfinderConfig));
    auto state = finder.makeState(mctx);
    VertexFinderOptions vfOptions(gctx, mctx);

    // Obtain PV
    std::vector<Acts::InputTrack> allInputTracks;
    for (auto& tp : allTrackParameters) {
      allInputTracks.emplace_back(&tp);
    }

    std::vector<Acts::Vertex> vertices;
    auto result = finder.find(allInputTracks, vfOptions, state);
    if (result.ok()) {
      vertices = std::move(result.value());
    }

    // Look for SV only if there are PVs
    if (vertices.size() != 0) {
      // Obtain the associated SV vertices
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

          // Note: should this be inside the previous if statement ?
          storeSecondaryVertices(vertices, verticesSec, *recotracks, *outputVertices, 
                                 mctx, gctx, ImPoEs_state, ipEst, vertexType);

          inputTracks.clear();
        }
      }
    }

    allInputTracks.clear();
    vertices.clear();
  }
}

} // namespace eicrecon
