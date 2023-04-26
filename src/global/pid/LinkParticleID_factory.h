// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// Link ParticleID objects with reconstructed particles

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>

// algorithms
#include <algorithms/pid/LinkParticleID.h>

// services
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

// other
#include <type_traits>

namespace eicrecon {

  class LinkParticleID;

  // NOTE: this is a template, to support an arbitrary particle datatype `ParticleDatatype`, such
  // as `edm4*::ReconstructedParticle` or `edm4*::MCRecoParticleAssociation`
  template<class ParticleDatatype>
  class LinkParticleID_factory :
    public JChainFactoryT<ParticleDatatype, LinkParticleIDConfig>,
    public SpdlogMixin<LinkParticleID_factory<ParticleDatatype>>
  {

    public:

      explicit LinkParticleID_factory(
          std::vector<std::string> default_input_tags,
          LinkParticleIDConfig cfg
          ):
        JChainFactoryT<ParticleDatatype, LinkParticleIDConfig>(std::move(default_input_tags), cfg) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

      eicrecon::LinkParticleID m_algo;
      std::string              m_detector_name;

  };
}
