// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

#include "algorithms/tracking/TrackParamTruthInit.h"
#include "algorithms/tracking/TrackParamTruthInitConfig.h"

namespace eicrecon {

    class TrackParamTruthInit_factory :
            public JChainFactoryT<edm4eic::TrackParameters, TrackParamTruthInitConfig>,
            public SpdlogMixin<TrackParamTruthInit_factory> {

    public:
        TrackParamTruthInit_factory( std::vector<std::string> default_input_tags, TrackParamTruthInitConfig cfg):
            JChainFactoryT<edm4eic::TrackParameters, TrackParamTruthInitConfig>(std::move(default_input_tags), cfg) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::TrackParamTruthInit m_seeding_algo;
    };

} // eicrecon
