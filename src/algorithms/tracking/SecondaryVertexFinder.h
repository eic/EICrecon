// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Dongwi H. Dongwi (Bishoy)

#pragma once

#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Utilities/Delegate.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Vertexing/IVertexFinder.hpp>
#include <Acts/Vertexing/TrackAtVertex.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <edm4eic/Cov4f.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <spdlog/logger.h>
#include <Eigen/Core>
#include <limits>
#include <math.h>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "Acts/Definitions/Units.hpp"
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"
#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include <algorithms/algorithm.h>

namespace eicrecon{
using SecondaryVertexFinderAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ReconstructedParticleCollection,std::vector<ActsExamples::Trajectories>>,
    algorithms::Output<edm4eic::VertexCollection,edm4eic::VertexCollection>>;
class SecondaryVertexFinder
    : public SecondaryVertexFinderAlgorithm{
public:
  SecondaryVertexFinder()
    : SecondaryVertexFinderAlgorithm{{"inputActsReconstructedParticles"},
                                     {"inputActsTrajectories"},
                                     {"outputPrimaryVertexAMVF"},
                                     {"outputSecondaryVertexAMVF"}} {}
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);

  std::tuple<std::unique_ptr<edm4eic::VertexCollection>, std::unique_ptr<edm4eic::VertexCollection>>
  produce(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

  // Calculate an initial Primary Vertex
  std::unique_ptr<edm4eic::VertexCollection>
  calculatePrimaryVertex(const edm4eic::ReconstructedParticleCollection*,
                 std::vector<const ActsExamples::Trajectories*> trajectories,
                 Acts::AdaptiveMultiVertexFinder&, Acts::VertexingOptions,
                 Acts::AdaptiveMultiVertexFinder::Config&, Acts::IVertexFinder::State&);

  //Calculate secondary vertex and store secVertex container
  std::unique_ptr<edm4eic::VertexCollection>
  calculateSecondaryVertex(const edm4eic::ReconstructedParticleCollection*,
             std::vector<const ActsExamples::Trajectories*> trajectories,
             Acts::AdaptiveMultiVertexFinder&, Acts::VertexingOptions,
             Acts::AdaptiveMultiVertexFinder::Config&, Acts::IVertexFinder::State&);

  // Functions to be used to check efficacy of sec. vertex
  std::unique_ptr<edm4eic::VertexCollection>
  getSecondaryVertex(const edm4eic::Vertex*, const TLorentzVector&, const edm4eic::TrackParameters,
            const edm4eic::TrackParameters, std::vector<double>&);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;
};

std::unique_ptr<edm4eic::VertexCollection> SecondaryVertexFinder::calculatePrimaryVertex(
    const edm4eic::ReconstructedParticleCollection* reconParticles,
    std::vector<const ActsExamples::Trajectories*> trajectories,
    Acts::AdaptiveMultiVertexFinder& finder, Acts::VertexingOptions finderOpts,
    Acts::AdaptiveMultiVertexFinder::Config& finderCfg, Acts::IVertexFinder::State& state) {

  auto prmVertices = std::make_unique<edm4eic::VertexCollection>();
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
  // -----> Fix: Use for later
  for (const auto& vtx : vertices) {
    edm4eic::Cov4f cov(vtx.fullCovariance()(0, 0), vtx.fullCovariance()(1, 1),
                       vtx.fullCovariance()(2, 2), vtx.fullCovariance()(3, 3),
                       vtx.fullCovariance()(0, 1), vtx.fullCovariance()(0, 2),
                       vtx.fullCovariance()(0, 3), vtx.fullCovariance()(1, 2),
                       vtx.fullCovariance()(1, 3), vtx.fullCovariance()(2, 3));
    auto eicvertex = prmVertices->create();
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
      const auto& par = finderCfg.extractParameters(t.originalParams);
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
            double EPSILON = std::numeric_limits<double>::epsilon()*1.0e-4; // mm
            if (std::abs((par.getLoc().a / edm4eic::unit::mm) - (loc_a / Acts::UnitConstants::mm)) <
                    EPSILON &&
                std::abs((par.getLoc().b / edm4eic::unit::mm) - (loc_b / Acts::UnitConstants::mm)) <
                    EPSILON) {
              m_log->trace(
                  "From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
                  par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
              eicvertex.addToAssociatedParticles(part);
            } // endif
          }   // end for par
        }     // end for trk
      }       // end for part
    }         // end for t
    m_log->debug("One AMVF vertex found at (x,y,z) = ({}, {}, {}) mm.",
                 vtx.position().x() / Acts::UnitConstants::mm,
                 vtx.position().y() / Acts::UnitConstants::mm,
                 vtx.position().z() / Acts::UnitConstants::mm);
  } // end for vtx
  return prmVertices;
}

std::unique_ptr<edm4eic::VertexCollection>
SecondaryVertexFinder::calculateSecondaryVertex(const edm4eic::ReconstructedParticleCollection* reconParticles,
                                  std::vector<const ActsExamples::Trajectories*> trajectories,
                                  Acts::AdaptiveMultiVertexFinder& vertexfinderSec,
                                  Acts::VertexingOptions vfOptions,
                                  Acts::AdaptiveMultiVertexFinder::Config& vertexfinderCfgSec,
                                  Acts::IVertexFinder::State& stateSec) {

  auto secVertices = std::make_unique<edm4eic::VertexCollection>();
  //--->Add Prm Vertex container here
  std::vector<Acts::InputTrack> inputTracks;
  for (unsigned int i = 0; i < trajectories.size(); i++) {
    auto tips = trajectories[i]->tips();
    if (tips.empty()) {
      continue;
    }
    for (unsigned int j = i + 1; j < trajectories.size(); j++) {
      auto tips2 = trajectories[j]->tips();
      if (tips2.empty()) {
        continue;
      }
      // Checking for default DCA cut-condition
      for (auto& tip : tips) {
        /// CKF can provide multiple track trajectories for a single input seed
        inputTracks.emplace_back(&(trajectories[i]->trackParameters(tip)));
      }
      for (auto& tip : tips2) {
        inputTracks.emplace_back(&(trajectories[j]->trackParameters(tip)));
      }
      // run the vertex finder for both tracks
      std::vector<Acts::Vertex> verticesSec;
      auto resultSecondary = vertexfinderSec.find(inputTracks, vfOptions, stateSec);
      if (resultSecondary.ok()) {
        verticesSec = std::move(resultSecondary.value());
      }

      for (const auto& secvertex : verticesSec) {
        edm4eic::Cov4f cov(secvertex.fullCovariance()(0, 0), secvertex.fullCovariance()(1, 1),
                           secvertex.fullCovariance()(2, 2), secvertex.fullCovariance()(3, 3),
                           secvertex.fullCovariance()(0, 1), secvertex.fullCovariance()(0, 2),
                           secvertex.fullCovariance()(0, 3), secvertex.fullCovariance()(1, 2),
                           secvertex.fullCovariance()(1, 3), secvertex.fullCovariance()(2, 3));
        auto eicvertex = secVertices->create();
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
          const auto& par = vertexfinderCfgSec.extractParameters(t.originalParams);
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
                double EPSILON = std::numeric_limits<double>::epsilon()*1.0e-4; // mm
                if (std::abs((par.getLoc().a / edm4eic::unit::mm) - (loc_a / Acts::UnitConstants::mm)) <
                        EPSILON &&
                    std::abs((par.getLoc().b / edm4eic::unit::mm) - (loc_b / Acts::UnitConstants::mm)) <
                        EPSILON) {
                  m_log->trace(
                      "From ReconParticles, track local position [Loc a, Loc b] = {} mm, {} mm",
                      par.getLoc().a / edm4eic::unit::mm, par.getLoc().b / edm4eic::unit::mm);
                  eicvertex.addToAssociatedParticles(part);
                } // endif
              }   // end for par
            }     // end for trk
          }       // end for part
        }         // end for t
      } // end for secvertex

      // empty the vector for the next set of tracks
      inputTracks.clear();
    } //end of int j=i+1
  }   // end of int i=0; i<trajectories.size()
  return secVertices;
}
} // namespace eicrecon
