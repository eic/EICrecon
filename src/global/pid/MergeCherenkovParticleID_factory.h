// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// Merge CherenkovParticleID results from each radiator, for a given Cherenkov PID subsystem

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <algorithm>
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
#include "extensions/jana/JChainFactoryT.h"
// services
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class MergeCherenkovParticleID_factory :
    public JChainFactoryT<edm4eic::CherenkovParticleID, MergeParticleIDConfig>,
    public SpdlogMixin
  {

    public:

      explicit MergeCherenkovParticleID_factory(
          std::vector<std::string> default_input_tags,
          MergeParticleIDConfig cfg
          ):
        JChainFactoryT<edm4eic::CherenkovParticleID, MergeParticleIDConfig>(std::move(default_input_tags), cfg) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      eicrecon::MergeParticleID m_algo;

  };
}
