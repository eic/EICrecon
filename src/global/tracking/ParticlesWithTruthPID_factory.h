// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/ReconstructedParticle.h>
#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <algorithms/tracking/ParticlesWithTruthPID.h>
#include <algorithms/tracking/ParticlesWithTruthPIDConfig.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class ParticlesWithTruthPID_factory :
            public JChainFactoryT<ParticlesWithAssociation, ParticlesWithTruthPIDConfig>,
            public SpdlogMixin<ParticlesWithTruthPID_factory> {

    public:
        explicit ParticlesWithTruthPID_factory( std::vector<std::string> default_input_tags, ParticlesWithTruthPIDConfig cfg):
            JChainFactoryT<ParticlesWithAssociation, ParticlesWithTruthPIDConfig>(std::move(default_input_tags), cfg) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::ParticlesWithTruthPID m_matching_algo;

    };

} // eicrecon

