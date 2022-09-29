// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKERHITCOLLECTOR_FACTORY_H
#define EICRECON_TRACKERHITCOLLECTOR_FACTORY_H

#include <spdlog/logger.h>
#include <edm4eic/TrackerHit.h>
#include <extensions/jana/JChainFactoryT.h>


namespace eicrecon {

    /// This factory just collects reconstructed hits edm4eic::TrackerHit from different sources
    /// And makes a single array out of them
    class TrackerHitCollector_factory : public JChainFactoryT<edm4eic::TrackerHit> {

    public:
        TrackerHitCollector_factory( std::vector<std::string> default_input_tags ):
        JChainFactoryT<edm4eic::TrackerHit>(std::move(default_input_tags) ) {
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

#endif //EICRECON_TRACKERHITCOLLECTOR_FACTORY_H
