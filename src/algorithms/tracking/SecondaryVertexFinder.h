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
#include <edm4eic/Cov4f.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <math.h>
#include <podio/RelationRange.h>
#include <spdlog/logger.h>
#include <Eigen/Core>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include <set>
#include <algorithm>

#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/LinearizedTrack.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include "Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Vertexing/AdaptiveMultiVertexFinder.hpp"
#include "ActsExamples/EventData/Trajectories.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
class SecondaryVertexFinder : public WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);

  std::tuple<std::unique_ptr<edm4eic::VertexCollection>, std::unique_ptr<edm4eic::VertexCollection>>
  produce(const edm4eic::ReconstructedParticleCollection*,
          std::vector<const ActsExamples::Trajectories*> trajectories);

  // Calculate an initial Primary Vertex
  std::unique_ptr<edm4eic::VertexCollection>
  calculatePrimaryVertex(const edm4eic::ReconstructedParticleCollection*,
                         std::vector<const ActsExamples::Trajectories*> trajectories,
                         Acts::EigenStepper<>);

  //Calculate secondary vertex and store secVertex container
  std::unique_ptr<edm4eic::VertexCollection>
  calculateSecondaryVertex(const edm4eic::ReconstructedParticleCollection*,
                           std::vector<const ActsExamples::Trajectories*> trajectories,
                           Acts::EigenStepper<>);

  // Functions to be used to check efficacy of sec. vertex
  void setVertexContainer(std::vector<Acts::Vertex> inputcontainer){vtx_container=inputcontainer;};

  //set up Impact estimator
  using ImpactPointEstimator   = Acts::ImpactPointEstimator;
  using LinearizerSec          = Acts::HelicalTrackLinearizer;
  using VertexFitterSec        = Acts::AdaptiveMultiVertexFitter;
  using VertexFinderSec        = Acts::AdaptiveMultiVertexFinder;
  using VertexFinderOptionsSec = Acts::VertexingOptions;
  using seedFinder = Acts::AdaptiveGridDensityVertexFinder;

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;
  std::vector<Acts::Vertex> vtx_container;
};

} // namespace eicrecon
