// Created by Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/Measurement2DCollection.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackerMeasurementFromHits.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class TrackerMeasurementFromHits_factory :
      public JChainMultifactoryT<NoConfig>,
      public SpdlogMixin {

    public:
        explicit TrackerMeasurementFromHits_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags)
        : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {
            DeclarePodioOutput<edm4eic::Measurement2D>(GetOutputTags()[0]);
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::TrackerMeasurementFromHits m_measurement;      /// Tracker measurement algorithm
    };

} // eicrecon
