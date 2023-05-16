// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/logger.h>
#include <edm4eic/TrackParametersCollection.h>
#include <extensions/jana/JChainFactoryT.h>
#include <algorithms/tracking/TrackerHitReconstructionConfig.h>


namespace eicrecon {

    /// This factory just collects reconstructed hits edm4eic::TrackerHit from different sources
    /// And makes a single array out of them
    class TrackerParticleCollector_factory : public JChainFactoryT<edm4eic::TrackParameters, TrackerHitReconstructionConfig> {

    public:
        TrackerParticleCollector_factory( std::vector<std::string> default_input_tags, TrackerHitReconstructionConfig cfg ):
                JChainFactoryT<edm4eic::TrackParameters, TrackerHitReconstructionConfig>(std::move(default_input_tags), cfg ) {
            // TrackerParticleCollector merges existing hits from different collections. We make this a subset collection
            // so that PODIO understands that this collection doesn't own its contents.
            SetSubsetCollection(true);
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
        std::vector<std::string> m_input_tags;              /// Tags of factories that provide input data

    };

} // eicrecon
