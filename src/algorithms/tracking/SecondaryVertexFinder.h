// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFinder.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/VertexCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SecondaryVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SecondaryVertexFinderAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            std::vector<const ActsExamples::Trajectories>>,
                          algorithms::Output<edm4eic::VertexCollection, edm4eic::VertexCollection>>;

class SecondaryVertexFinder : public SecondaryVertexFinderAlgorithm,
                              public WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  SecondaryVertexFinder(std::string_view name)
      : SecondaryVertexFinderAlgorithm{name,
                                       {"inputReconstructedParticles", "inputActsTrajectories"},
                                       {"outputPrimaryVertices", "outputSecondaryVertices"},
                                       ""} {}

  void init(std::shared_ptr<spdlog::logger> log) /*final*/;

  void process(const Input&, const Output&) const final;

  // Calculate an initial Primary Vertex
  void calculatePrimaryVertex(
      const edm4eic::ReconstructedParticleCollection&,
      const std::vector<gsl::not_null<const ActsExamples::Trajectories*>>& trajectories,
      Acts::EigenStepper<>, edm4eic::VertexCollection&) const;

  //Calculate secondary vertex and store secVertex container
  void calculateSecondaryVertex(
      const edm4eic::ReconstructedParticleCollection&,
      const std::vector<gsl::not_null<const ActsExamples::Trajectories*>>& trajectories,
      Acts::EigenStepper<>, edm4eic::VertexCollection&) const;

  // Functions to be used to check efficacy of sec. vertex
  void setVertexContainer(std::vector<Acts::Vertex> inputcontainer) {
    vtx_container = inputcontainer;
  };

  //set up Impact estimator
  using ImpactPointEstimator   = Acts::ImpactPointEstimator;
  using LinearizerSec          = Acts::HelicalTrackLinearizer;
  using VertexFitterSec        = Acts::AdaptiveMultiVertexFitter;
  using VertexFinderSec        = Acts::AdaptiveMultiVertexFinder;
  using VertexFinderOptionsSec = Acts::VertexingOptions;
  using seedFinder             = Acts::AdaptiveGridDensityVertexFinder;

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
