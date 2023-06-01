// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

#include <algorithms/tracking/JugTrack/Track.hpp>


namespace eicrecon {

class TrackParamSeeding_factory :
        public JChainFactoryT<eicrecon::TrackParameters, NoConfig, JFactoryT>,
        public SpdlogMixin<TrackParamSeeding_factory>  {

    public:
        TrackParamSeeding_factory( std::vector<std::string> default_input_tags ):
                JChainFactoryT<eicrecon::TrackParameters, NoConfig, JFactoryT>(std::move(default_input_tags) ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

    };

} // eicrecon

