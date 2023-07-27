// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// Merge CherenkovParticleID results from each radiator, for a given Cherenkov PID subsystem

#pragma once

// JANA
#include "extensions/jana/JChainFactoryT.h"
#include <JANA/JEvent.h>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>

// algorithms
#include "algorithms/pid/MergeParticleID.h"

// services
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class MergeParticleID;

  class MergeCherenkovParticleID_factory :
    public JChainFactoryT<edm4eic::CherenkovParticleID, MergeParticleIDConfig>,
    public SpdlogMixin<MergeCherenkovParticleID_factory>
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
