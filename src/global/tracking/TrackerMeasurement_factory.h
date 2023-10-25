// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/Measurement2D.h>
#include <edm4eic/Measurement2DCollection.h>
#include <spdlog/logger.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackerMeasurement.h"
#include "extensions/jana/JChainFactoryT.h"

namespace eicrecon {

    class TrackerMeasurement_factory : public JChainFactoryT<edm4eic::Measurement2D, NoConfig>{

    public:
        TrackerMeasurement_factory( std::vector<std::string> default_input_tags):
                JChainFactoryT<edm4eic::Measurement2D, NoConfig>(std::move(default_input_tags) ) {
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
        eicrecon::TrackerMeasurement m_measurement;      /// Tracker measurement algorithm
    };

} // eicrecon
