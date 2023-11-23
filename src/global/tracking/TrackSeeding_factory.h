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

#include "OrthogonalTrackSeedingConfig.h"
#include "algorithms/tracking/TrackSeeding.h"
#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

    class TrackSeeding_factory :
            public JChainFactoryT<edm4eic::TrackParameters, OrthogonalTrackSeedingConfig>,
            public SpdlogMixin {

    public:
        TrackSeeding_factory( std::vector<std::string> default_input_tags, OrthogonalTrackSeedingConfig cfg):
                JChainFactoryT<edm4eic::TrackParameters, OrthogonalTrackSeedingConfig>(std::move(default_input_tags), cfg ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::TrackSeeding m_seeding_algo;                      /// Proxy tracking algorithm

    };

} // eicrecon
