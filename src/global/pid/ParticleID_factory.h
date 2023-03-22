// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>

// algorithms
#include <algorithms/pid/ParticleID.h>

// services
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class ParticleID;
  class ParticleID_factory :
    public JChainFactoryT<edm4hep::ParticleID, ParticleIDConfig>,
    public SpdlogMixin<ParticleID_factory>
  {

    public:

      explicit ParticleID_factory(std::vector<std::string> default_input_tags, ParticleIDConfig cfg) :
        JChainFactoryT<edm4hep::ParticleID, ParticleIDConfig>(std::move(default_input_tags), cfg) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      eicrecon::ParticleID m_algo;
      std::string          m_detector_name;

  };
}
