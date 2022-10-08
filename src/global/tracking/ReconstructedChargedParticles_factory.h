// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_RECONSTRUCTEDCHARGEDPARTICLES_FACTORY_H
#define EICRECON_RECONSTRUCTEDCHARGEDPARTICLES_FACTORY_H

#include <edm4eic/ReconstructedParticle.h>
#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <algorithms/tracking/ParticlesWithTruthPID.h>
#include <algorithms/tracking/ParticlesWithTruthPIDConfig.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class ReconstructedChargedParticles_factory:
            public JChainFactoryT<edm4eic::ReconstructedParticle>,
            public SpdlogMixin<ReconstructedChargedParticles_factory> {

    public:
        explicit ReconstructedChargedParticles_factory( std::vector<std::string> default_input_tags):
            JChainFactoryT<edm4eic::ReconstructedParticle>(std::move(default_input_tags)) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    };

} // eicrecon

#endif //EICRECON_RECONSTRUCTEDCHARGEDPARTICLES_FACTORY_H
