// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/pid/ParticlesWithPID.h"
#include "algorithms/pid/ParticlesWithPIDConfig.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

    class ParticlesWithPID_factory :
            public JChainMultifactoryT<ParticlesWithPIDConfig>,
            public SpdlogMixin {

    public:
        explicit ParticlesWithPID_factory( std::string tag,
                                                const std::vector<std::string>& input_tags,
                                                const std::vector<std::string>& output_tags,
                                                ParticlesWithPIDConfig cfg):
            JChainMultifactoryT<ParticlesWithPIDConfig>(std::move(tag), input_tags, output_tags, cfg) {

            if (GetOutputTags().size() != 3)
              throw JException("incorrect number of input tags");
            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags().at(0));
            DeclarePodioOutput<edm4eic::MCRecoParticleAssociation>(GetOutputTags().at(1));
            DeclarePodioOutput<edm4hep::ParticleID>(GetOutputTags().at(2));
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::ParticlesWithPID m_matching_algo;
    };

} // eicrecon
