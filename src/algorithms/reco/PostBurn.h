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

  using PostBurnAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::MCParticleCollection
    >,
    algorithms::Output<
      edm4hep::MCParticleCollection
    >
  >;

  class PostBurn
  : public PostBurnAlgorithm,
    public WithPodConfig<PostBurnConfig> {

  public:
    PostBurn(std::string_view name)
      : PostBurnAlgorithm{name,
                            {"inputMCParticles"},
                            {"outputMCParticles"},
                            "Apply boosts and rotations to remove crossing angle and beam effects."} {}

    void init();
    void process(const Input&, const Output&) const final;

  private:


  };
}
