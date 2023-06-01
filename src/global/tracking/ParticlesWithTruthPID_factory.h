// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/ReconstructedParticle.h>
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <algorithms/tracking/ParticlesWithTruthPID.h>
#include <algorithms/tracking/ParticlesWithTruthPIDConfig.h>
#include <spdlog/logger.h>


namespace eicrecon {

    class ParticlesWithTruthPID_factory :
            public JChainMultifactoryT<ParticlesWithTruthPIDConfig>,
            public SpdlogMixin<ParticlesWithTruthPID_factory> {

    public:
        explicit ParticlesWithTruthPID_factory( std::string tag,
                                                const std::vector<std::string>& input_tags,
                                                const std::vector<std::string>& output_tags,
                                                ParticlesWithTruthPIDConfig cfg):
            JChainMultifactoryT<ParticlesWithTruthPIDConfig>(std::move(tag), input_tags, output_tags, cfg) {

            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
            DeclarePodioOutput<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1]);
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::ParticlesWithTruthPID m_matching_algo;
    };

} // eicrecon
