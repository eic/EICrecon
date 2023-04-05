// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// Link ParticleID objects with ReconstructedParticles

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>

// algorithms
#include <algorithms/pid/LinkParticleID.h>

// services
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class LinkParticleID;
  class LinkParticleID_factory :
    public JChainFactoryT<edm4eic::ReconstructedParticle, LinkParticleIDConfig>,
    public SpdlogMixin<LinkParticleID_factory>
  {

    public:

      explicit LinkParticleID_factory(std::vector<std::string> default_input_tags, LinkParticleIDConfig cfg) :
        JChainFactoryT<edm4eic::ReconstructedParticle, LinkParticleIDConfig>(std::move(default_input_tags), cfg) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      eicrecon::LinkParticleID m_algo;
      std::string              m_detector_name;

  };
}
