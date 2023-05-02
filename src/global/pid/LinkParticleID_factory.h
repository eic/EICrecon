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

  /* NOTE: this factory is a template, to support an arbitrary particle
   * datatype `ParticleDataT`, with corresponding collection type `CollectionT`
   * - see the end of implementation (LinkParticleID_factory.cpp) for supported
   *   datatypes, specified as expicit instantiations
   */
  template<class ParticleDataT, class ParticleCollectionT>
  class LinkParticleID_factory :
    public JChainFactoryT<ParticleDataT, LinkParticleIDConfig>,
    public SpdlogMixin<LinkParticleID_factory<ParticleDataT,ParticleCollectionT>>
  {

    public:

      explicit LinkParticleID_factory(
          std::vector<std::string> default_input_tags,
          LinkParticleIDConfig cfg
          ):
        JChainFactoryT<ParticleDataT, LinkParticleIDConfig>(std::move(default_input_tags), cfg) {}

      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

      eicrecon::LinkParticleID m_algo;
      std::string              m_detector_name;

  };

}
