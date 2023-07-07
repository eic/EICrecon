// Copyright (C) 2022, 2023 Daniel Brandenburg
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// algorithms
#include <algorithms/reco/ElectronReconstruction.h>

// services
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>

namespace eicrecon {

  class ReconstructedElectrons_factory :
    public JChainFactoryT<edm4eic::ReconstructedParticle>,
    public SpdlogMixin<ReconstructedElectrons_factory>
  {

    public:

      explicit ReconstructedElectrons_factory(std::vector<std::string> default_input_tags) :
        JChainFactoryT<edm4eic::ReconstructedParticle>(std::move(default_input_tags)) {
        }

      /** One time initialization **/
      void Init() override {
        // get plugin name and tag
        auto app    = GetApplication();
        auto plugin = GetPluginName();
        auto prefix = plugin + ":" + GetTag();
        InitDataTags(prefix);

        // services
        InitLogger(prefix, "info");
        m_algo.init(m_log);
      }

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override{}

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override{
        // Step 1. lets collect the Cluster associations from various detectors
        std::vector<std::vector<const edm4eic::MCRecoClusterParticleAssociation*>> in_clu_assoc;
        for(auto& input_tag : GetInputTags()){
          // only collect from the sources that provide ClusterAssociations
          if ( input_tag.find( "ClusterAssociations" ) == std::string::npos ) {
            continue;
          }
          m_log->debug( "Adding cluster associations from: {}", input_tag );
          in_clu_assoc.push_back(
            event->Get<edm4eic::MCRecoClusterParticleAssociation>(input_tag)
          );
        }

        // Step 2. Get MC, RC, and MC-RC association info
        // This is needed as a bridge to get RecoCluster - RC Particle associations
        auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
        auto rc_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");
        auto rc_particles_assoc = event->Get<edm4eic::MCRecoParticleAssociation>("ReconstructedChargedParticleAssociations");

        // Step 3. Pass everything to "the algorithm"
        // in the future, select appropriate algorithm (truth, fully reco, etc.)
        auto output = m_algo.execute(
          mc_particles,
          rc_particles,
          rc_particles_assoc,
          in_clu_assoc
        );

        m_log->debug( "We have found {} reconstructed electron candidates this event", output.size() );

        // Step 4. Output the collection
        Set( std::move(output) );
      }

    private:

      // underlying algorithm
      eicrecon::ElectronReconstruction m_algo;
  };
}
