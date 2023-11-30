// Copyright (C) 2022, 2023 Daniel Brandenburg
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include "extensions/jana/JChainMultifactoryT.h"
#include <JANA/JEvent.h>

// algorithms
#include "algorithms/reco/ElectronReconstruction.h"

// services
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class ReconstructedElectrons_factory :
    public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin
  {

    public:

      explicit ReconstructedElectrons_factory(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags)
      : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {
          DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
      }

      /** One time initialization **/
      void Init() override {
        // get plugin name and tag
        auto app    = GetApplication();
        auto plugin = GetPluginName();
        auto prefix = plugin + ":" + GetTag();

        // services
        InitLogger(app, prefix, "info");
        m_algo.init(m_log);
      }

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override{
        // Step 1. lets collect the Cluster associations from various detectors
        std::vector<const edm4eic::MCRecoClusterParticleAssociationCollection*> in_clu_assoc;
        for(auto& input_tag : GetInputTags()){
          // only collect from the sources that provide ClusterAssociations
          if ( input_tag.find( "ClusterAssociations" ) == std::string::npos ) {
            continue;
          }
          m_log->trace( "Adding cluster associations from: {}", input_tag );
          in_clu_assoc.push_back(
            static_cast<const edm4eic::MCRecoClusterParticleAssociationCollection*>(event->GetCollectionBase(input_tag))
          );
        }

        // Step 2. Get MC, RC, and MC-RC association info
        // This is needed as a bridge to get RecoCluster - RC Particle associations

        auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase("MCParticles"));
        auto rc_particles = static_cast<const edm4eic::ReconstructedParticleCollection*>(event->GetCollectionBase("ReconstructedChargedParticles"));
        auto rc_particles_assoc = static_cast<const edm4eic::MCRecoParticleAssociationCollection*>(event->GetCollectionBase("ReconstructedChargedParticleAssociations"));

        // Step 3. Pass everything to "the algorithm"
        // in the future, select appropriate algorithm (truth, fully reco, etc.)
        auto output = m_algo.execute(
          mc_particles,
          rc_particles,
          rc_particles_assoc,
          in_clu_assoc
        );

        m_log->debug( "We have found {} reconstructed electron candidates this event", output->size() );
        // Step 4. Output the collection
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(output));
      }

    private:

      // underlying algorithm
      eicrecon::ElectronReconstruction m_algo;
  };
}
