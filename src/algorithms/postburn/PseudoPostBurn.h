// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "PostBurnConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using PseudoPostBurnAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::MCParticleCollection,
      edm4eic::ReconstructedParticleCollection,
	  edm4eic::MCRecoParticleAssociationCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection
    >
  >;

  class PseudoPostBurn
  : public PseudoPostBurnAlgorithm,
    public WithPodConfig<PostBurnConfig> {

  public:
    PseudoPostBurn(std::string_view name)
      : PseudoPostBurnAlgorithm{name,
                            {"mcParticles", "ReconstructedParticleCollection", "MCRecoParticleAssociationCollection"},
                            {"outputParticleCollection"},
                            "Apply boosts and rotations to remove crossing angle and beam effects."} {}

    void init(std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:

    /** algorithm logger */
    std::shared_ptr<spdlog::logger>   m_log;

  };
}
