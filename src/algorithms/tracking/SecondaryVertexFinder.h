// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <Acts/Vertexing/Vertex.hpp>
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
    algorithms::Input<edm4eic::ReconstructedParticleCollection, ActsPodioEdm::TrackStateCollection,
                      ActsPodioEdm::BoundParametersCollection, ActsPodioEdm::JacobianCollection,
                      ActsPodioEdm::TrackCollection>,
    algorithms::Output<edm4eic::VertexCollection>>;

class SecondaryVertexFinder : public SecondaryVertexFinderAlgorithm,
                              public WithPodConfig<eicrecon::SecondaryVertexFinderConfig> {
public:
  SecondaryVertexFinder(std::string_view name)
      : SecondaryVertexFinderAlgorithm{
            name,
            {"inputReconstructedParticles", "inputActsTrackStates", "inputActsTrackParameters",
             "inputActsTrackJacobians", "inputActsTracks"},
            {"outputVertices"},
            "Finds vertices using ACTS Adaptive Multi-Vertex Finder (AMVF)"} {}

  void init() final {};

  void process(const Input&, const Output&) const final;

private:
  /// Store found ACTS vertices into the EDM4eic vertex collection.
  void storeVertices(const std::vector<Acts::Vertex>& vertices,
                     const edm4eic::ReconstructedParticleCollection& reconParticles,
                     edm4eic::VertexCollection& outputVertices, int vertexType) const;

  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{
      algorithms::ActsSvc::instance().acts_geometry_provider()};
  std::shared_ptr<const Acts::MagneticFieldProvider> m_BField{m_geoSvc->getFieldProvider()};
};

} // namespace eicrecon
