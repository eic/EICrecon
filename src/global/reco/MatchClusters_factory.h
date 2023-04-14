// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/ReconstructedParticle.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <algorithms/reco/MatchClusters.h>



namespace eicrecon {

    class MatchClusters_factory :
            public JChainMultifactoryT<NoConfig>,
            public SpdlogMixin<MatchClusters_factory> {

    public:
        explicit MatchClusters_factory(std::string tag,
                                       const std::vector<std::string>& input_tags,
                                       const std::vector<std::string>& output_tags):
                JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

            // TODO: NWB: object ownership is set to false because right now we populate the collections without doing any matching
            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0], false);
            DeclarePodioOutput<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1], false);
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
