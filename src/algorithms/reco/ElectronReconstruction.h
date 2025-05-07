// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg

#pragma once

#include <memory>

#include "ElectronReconstructionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace edm4eic {
class ReconstructedParticleCollection;
}
namespace spdlog {
class logger;
}

namespace eicrecon {

class ElectronReconstruction : public WithPodConfig<ElectronReconstructionConfig> {

public:
  void init(std::shared_ptr<spdlog::logger> logger);

  // idea will be to overload this with other version (e.g. reco mode)
  std::unique_ptr<edm4eic::ReconstructedParticleCollection>
  execute(const edm4eic::ReconstructedParticleCollection* rcparts);

private:
  std::shared_ptr<spdlog::logger> m_log;
};
} // namespace eicrecon
