// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackProjector.h"
#include "algorithms/tracking/TrackProjectorConfig.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class TrackProjector_factory:
    public JChainMultifactoryT<TrackProjectorConfig>,
            public SpdlogMixin {

    public:
        explicit TrackProjector_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags,
            TrackProjectorConfig cfg)
        : JChainMultifactoryT<TrackProjectorConfig>(std::move(tag), input_tags, output_tags, cfg) {
            DeclarePodioOutput<edm4eic::TrackSegment>(GetOutputTags()[0]);
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::TrackProjector m_track_projector_algo;

    };

} // eicrecon
