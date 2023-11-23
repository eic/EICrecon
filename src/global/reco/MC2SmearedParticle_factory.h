// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "algorithms/reco/MC2SmearedParticle.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class MC2SmearedParticle_factory:
            public JChainMultifactoryT<NoConfig>,
            public SpdlogMixin {
    public:

        explicit MC2SmearedParticle_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags)
        : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {
            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::MC2SmearedParticle m_smearing_algo;       /// Actual digitisation algorithm

    };

} // eicrecon
