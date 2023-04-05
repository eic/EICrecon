// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/ReconstructedParticle.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/tracking/TrackProjector.h>
#include <algorithms/tracking/TrackProjectorConfig.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class TrackProjector_factory:
    public JChainFactoryT<edm4eic::TrackSegment, TrackProjectorConfig>,
            public SpdlogMixin<TrackProjector_factory> {

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
