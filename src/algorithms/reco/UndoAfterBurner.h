// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#include <algorithms/algorithm.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "UndoAfterBurnerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using UndoAfterBurnerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4hep::MCParticleCollection>>;

class UndoAfterBurner : public UndoAfterBurnerAlgorithm,
                        public WithPodConfig<UndoAfterBurnerConfig> {

public:
  UndoAfterBurner(std::string_view name)
      : UndoAfterBurnerAlgorithm{
            name,
            {"inputMCParticles"},
            {"outputMCParticles"},
            "Apply boosts and rotations to remove crossing angle and beam effects."} {}

  void init();
  void process(const Input&, const Output&) const final;

private:
};
} // namespace eicrecon
