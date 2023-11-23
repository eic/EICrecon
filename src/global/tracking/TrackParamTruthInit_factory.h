// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/TrackParametersCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackParamTruthInit.h"
#include "algorithms/tracking/TrackParamTruthInitConfig.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class TrackParamTruthInit_factory :
            public JChainMultifactoryT<TrackParamTruthInitConfig>,
            public SpdlogMixin {

    public:
        explicit TrackParamTruthInit_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags,
            TrackParamTruthInitConfig cfg)
        : JChainMultifactoryT<TrackParamTruthInitConfig>(std::move(tag), input_tags, output_tags, cfg) {
            DeclarePodioOutput<edm4eic::TrackParameters>(GetOutputTags()[0]);
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::TrackParamTruthInit m_seeding_algo;
    };

} // eicrecon
