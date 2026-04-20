// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/VertexCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "ActsGeometryProvider.h"
#include "SecondaryVertexFinderConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SecondaryVertexFinderAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            ActsExamples::ConstTrackContainer>,
                          algorithms::Output<edm4eic::VertexCollection, edm4eic::VertexCollection>>;

class SecondaryVertexFinder : public SecondaryVertexFinderAlgorithm,
                              public WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  SecondaryVertexFinder(std::string_view name)
      : SecondaryVertexFinderAlgorithm{name,
                                       {"inputReconstructedParticles", "inputActsTracks"},
                                       {"outputPrimaryVertices", "outputSecondaryVertices"},
                                       ""} {}

  void init() final;

  void process(const Input&, const Output&) const final;

  // FIXME this is not compliant with algorithms interface
  void applyLogger(std::shared_ptr<spdlog::logger> log) { m_log = log; };

private:
  // Calculate an initial Primary Vertex
  void calculatePrimaryVertex(
      const edm4eic::ReconstructedParticleCollection&,
      const ActsExamples::ConstTrackContainer* constTracks,
      Acts::EigenStepper<>, edm4eic::VertexCollection&) const;

  //Calculate secondary vertex and store secVertex container
  void calculateSecondaryVertex(
      const edm4eic::ReconstructedParticleCollection&,
      const ActsExamples::ConstTrackContainer* constTracks,
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

  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const Acts::MagneticFieldProvider> m_BField = nullptr;
  Acts::GeometryContext m_geoctx;
  Acts::MagneticFieldContext m_fieldctx;
  SecondaryVertexFinderConfig m_cfg;
  std::vector<Acts::Vertex> vtx_container;
};

} // namespace eicrecon
