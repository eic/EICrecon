// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>
#include <extensions/jana/JChainFactoryT.h>
#include <algorithms/tracking/JugTrack/GeometryContainers.hpp>
#include <algorithms/tracking/TrackerSourceLinker.h>

#include <algorithms/tracking/TrackerSourceLinker.h>
#include <algorithms/tracking/ActsGeometryProvider.h>

namespace eicrecon {

    class TrackerSourceLinker;

    class TrackerSourceLinker_factory : public JChainFactoryT<TrackerSourceLinkerResult, NoConfig, JFactoryT> {

    public:
        TrackerSourceLinker_factory( std::vector<std::string> default_input_tags):
                JChainFactoryT<TrackerSourceLinkerResult, NoConfig, JFactoryT>(std::move(default_input_tags) ) {
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
        eicrecon::TrackerSourceLinker m_source_linker;      /// Track source linker algorithm
    };

} // eicrecon
