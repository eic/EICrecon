// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Vertexing/AdaptiveGridDensityVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFinder.hpp>
#include <Acts/Vertexing/AdaptiveMultiVertexFitter.hpp>
#include <Acts/Vertexing/HelicalTrackLinearizer.hpp>
#include <Acts/Vertexing/ImpactPointEstimator.hpp>
#include <Acts/Vertexing/Vertex.hpp>
#include <Acts/Vertexing/VertexingOptions.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/VertexCollection.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/SecondaryVertexFinderConfig.h"

namespace eicrecon {

using SecondaryVertexFinderAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ReconstructedParticleCollection, Acts::ConstVectorMultiTrajectory,
                      Acts::ConstVectorTrackContainer>,
    algorithms::Output<edm4eic::VertexCollection, edm4eic::VertexCollection>>;

class SecondaryVertexFinder : public SecondaryVertexFinderAlgorithm,
                              public WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  SecondaryVertexFinder(std::string_view name)
      : SecondaryVertexFinderAlgorithm{
            name,
            {"inputReconstructedParticles", "inputActsTrackStates", "inputActsTracks"},
            {"outputPrimaryVertices", "outputSecondaryVertices"},
            "Finds vertices using ACTS Adaptive Multi-Vertex Finder (AMVF)"} {}

  void init() final {};

  void process(const Input&, const Output&) const final;

private:
  // Calculate an initial Primary Vertex
  void calculatePrimaryVertex(const edm4eic::ReconstructedParticleCollection&,
                              const Acts::ConstVectorMultiTrajectory* trackStates,
                              const Acts::ConstVectorTrackContainer* tracks, Acts::EigenStepper<>,
                              edm4eic::VertexCollection&) const;

  //Calculate secondary vertex and store secVertex container
  void calculateSecondaryVertex(const edm4eic::ReconstructedParticleCollection&,
                                const Acts::ConstVectorMultiTrajectory* trackStates,
                                const Acts::ConstVectorTrackContainer* tracks, Acts::EigenStepper<>,
                                edm4eic::VertexCollection&) const;

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

  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{
      algorithms::ActsSvc::instance().acts_geometry_provider()};
  std::shared_ptr<const Acts::MagneticFieldProvider> m_BField{m_geoSvc->getFieldProvider()};
  std::vector<Acts::Vertex> vtx_container;
};

} // namespace eicrecon
