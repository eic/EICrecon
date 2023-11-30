// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// Merge CherenkovParticleID results from each radiator, for a given Cherenkov PID subsystem

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/pid/MergeParticleID.h"
#include "algorithms/pid/MergeParticleIDConfig.h"
// JANA
#include "extensions/jana/JChainMultifactoryT.h"
// services
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class MergeCherenkovParticleID_factory :
    public JChainMultifactoryT<MergeParticleIDConfig>,
    public SpdlogMixin
  {

    public:

      explicit MergeCherenkovParticleID_factory(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          MergeParticleIDConfig cfg)
      : JChainMultifactoryT<MergeParticleIDConfig>(std::move(tag), input_tags, output_tags, cfg) {
        DeclarePodioOutput<edm4eic::CherenkovParticleID>(GetOutputTags()[0]);
      }

      /** One time initialization **/
      void Init() override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      eicrecon::MergeParticleID m_algo;

  };
}
