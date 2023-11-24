// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/MatchClusters.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"



namespace eicrecon {

    class MatchClusters_factory :
            public JChainMultifactoryT<NoConfig>,
            public SpdlogMixin {

    public:
        explicit MatchClusters_factory(std::string tag,
                                       const std::vector<std::string>& input_tags,
                                       const std::vector<std::string>& output_tags):
                JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
            DeclarePodioOutput<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1]);
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void BeginRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;
    protected:

        std::vector<std::string> m_input_assoc_tags;
        MatchClusters m_match_algo;

    };

} // eicrecon
