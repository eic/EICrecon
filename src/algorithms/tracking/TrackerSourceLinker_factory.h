// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKERSOURCELINKER_FACTORY_H
#define EICRECON_TRACKERSOURCELINKER_FACTORY_H

#include <spdlog/spdlog.h>
#include "extensions/jana/JChainFactoryT.h"
#include <algorithms/tracking/TrackerSourceLinker.h>

#include "TrackSourceLinkerResult.h"
#include "GeoSvc.h"


namespace eicrecon {

    class TrackerSourceLinker_factory : public JChainFactoryT<TrackSourceLinkerResult> {

    public:
        TrackerSourceLinker_factory( std::vector<std::string> default_input_tags ):
                JChainFactoryT<TrackSourceLinkerResult>( std::move(default_input_tags) ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory

        int m_verbose;                                      /// verbosity 0-none, 1-default, 2-debug, 3-trace

        GeoSvc *m_acts_context;
        eicrecon::TrackerSourceLinker m_source_linker;      /// Track source linker algorithm

        std::vector<std::string> m_input_tags;              /// Tags of factories that provide input data
    };

} // eicrecon

#endif //EICRECON_TRACKERSOURCELINKER_FACTORY_H
