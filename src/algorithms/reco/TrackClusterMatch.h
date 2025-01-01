#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

  using TrackClusterMatchAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ClusterCollection,
      edm4eic::TrackSegmentCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection
    >
  >;

  class TrackClusterMatch : public TrackClusterMatchAlgorithm {
  public:

    TrackClusterMatch(std::string_view name)
      : TrackClusterMatchAlgorithm{name,
                            {"inputParticles", "inputElectronCandidates"},
                            {"outputElectrons"},
                            "Outputs DIS electrons ordered in decreasing E-pz"} {}
    void init() final;
    void process(const Input&, const Output&) const final;

  private:
    const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();

  };
}
