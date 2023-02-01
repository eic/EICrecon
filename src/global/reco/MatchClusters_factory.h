// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_MatchClusters_factory_H
#define EICRECON_MatchClusters_factory_H

#include <edm4eic/ReconstructedParticle.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <algorithms/reco/MatchClusters.h>
#include <algorithms/reco/ParticlesWithAssociation.h>


namespace eicrecon {

    class MatchClusters_factory :
            public JChainFactoryT<ParticlesWithAssociation>,
            public SpdlogMixin<MatchClusters_factory> {

    public:
        explicit MatchClusters_factory(std::vector<std::string> default_input_tags):
            JChainFactoryT<ParticlesWithAssociation>( std::move(default_input_tags)) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;
    protected:

        std::vector<std::string> m_input_assoc_tags;
        MatchClusters m_match_algo;

    };

} // eicrecon

#endif //EICRECON_MatchClusters_factory_H
