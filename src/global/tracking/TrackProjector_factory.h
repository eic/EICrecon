// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackProjector.h"
#include "algorithms/tracking/TrackProjectorConfig.h"
#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class TrackProjector_factory:
    public JChainFactoryT<edm4eic::TrackSegment, TrackProjectorConfig>,
            public SpdlogMixin {

    public:
        explicit TrackProjector_factory( std::vector<std::string> default_input_tags, TrackProjectorConfig cfg):
        JChainFactoryT<edm4eic::TrackSegment, TrackProjectorConfig>(std::move(default_input_tags), cfg) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
        eicrecon::TrackProjector m_track_projector_algo;

    };

} // eicrecon
