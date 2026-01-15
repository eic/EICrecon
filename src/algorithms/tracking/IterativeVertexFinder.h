// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/VertexCollection.h>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/IterativeVertexFinderConfig.h"

namespace eicrecon {

using IterativeVertexFinderAlgorithm = algorithms::Algorithm<
    algorithms::Input<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                      ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection,
                      edm4eic::ReconstructedParticleCollection>,
    algorithms::Output<edm4eic::VertexCollection>>;

class IterativeVertexFinder : public IterativeVertexFinderAlgorithm,
                              public WithPodConfig<eicrecon::IterativeVertexFinderConfig> {
public:
  IterativeVertexFinder(std::string_view name)
      : IterativeVertexFinderAlgorithm{name,
                                       {"inputActsTrackStates", "inputActsTrackParameters",
                                        "inputActsTrackJacobians", "inputActsTracks",
                                        "inputReconstructedParticles"},
                                       {"outputVertices"},
                                       "Iterative vertex finder"} {}

  void init() final {};
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};
  std::shared_ptr<const Acts::MagneticFieldProvider> m_BField{m_geoSvc->getFieldProvider()};
};
} // namespace eicrecon
